#ifndef SENSORCONTROL_H
#define SENSORCONTROL_H

#include <QtWidgets/QMainWindow>
#include <QMutex>
#include <QTime>

#include "ui_sensorcontrol.h"

#include "ftd2xx.h"

class CWaitingThread;
class QFile;

enum
{
	REG_WR=0,
	REG_RD=1
};

enum
{
	PKG_TYPE_INFO=0,
	PKG_TYPE_ERRORMES=1,
	PKG_TYPE_ASCIIMES=2,
	PKG_TYPE_RWSW=3,
	PKG_TYPE_RWPACKET=4,
	PKG_TYPE_RWHW=5
};

enum
{
	IMODE_16,
	IMODE_12,
	IMODE_8
};

enum
{
	FILE_RBF = 0,
	FILE_RAW = 1
};



class SensorControl : public QMainWindow
{
	Q_OBJECT

public:
	SensorControl(QWidget *parent = 0);
	~SensorControl();

protected:
	virtual void	closeEvent(QCloseEvent * event);

private:
	Ui::SensorControlClass ui;

	FT_HANDLE m_handle;
	HANDLE m_hEvent;
	CWaitingThread* m_waitingThread; 
	quint32 m_flashID;
	QMutex m_mtx;
	quint64 m_readBytes;
	
	quint64 m_inputLength;
	QFile* m_inputFile;	
	unsigned char m_buff[2048];
	QString m_fileName;
	int m_fileType;
	QString m_lastErrorStr;
	quint32 m_startAddr;
	quint32 m_temperature;
	quint32 m_R3;
	bool m_cancel;
	bool m_gettingFile;
	QTime m_time;

	QTimer* m_timer4WaitFrame;
	QString m_framesPath;
	int m_frameCnt;
	QTimer* m_timer;

	bool openPort(int aNum);
	void closePort();
	int waitForPacket(int& tt, int& aCode);
	QByteArray hexStringToByteArray(QString& aStr);
	void fillDeviceList();
	int sendPacket(unsigned char aType, quint16 aLen, unsigned char aRdWr,quint16 aAddr, quint32 aData = 0);
	bool setRbfFileName(const QString&);
//	void toLog(const QString& logStr);
	void setDebugMode(bool);

private slots:
	void updateFramesList();
	void slShowTerminal(bool);

	void slSend();
	void addDataToTE(const QString& str);
	void slOpen();
	void slClose();
	bool slGetInfo();
	void slNewKadr(unsigned char aType, unsigned short aLen, const unsigned char* aData);
	bool slBrowseRBF();
	bool slWriteFlash();
	bool slReadFlashID();
	bool slReadStartAddress();
	bool slEraseFlash();
	bool slWriteLength();
	bool slWriteCmdUpdateFirmware();
	void slReadFlash();
	void slUpdateFirmware();
	void slConnectToDevice();
	bool slJumpToFact();
	bool slJumpToAppl();
	void slCancelUpdate();
	void slViewRaw();
	void slDrawPicture(const QString& fileName);

	void slWaitFrameFinished();

	void slSelectedFrame(QListWidgetItem *,QListWidgetItem *);
	void slClearFrameFolder();
	void slOpenFolder();

	void slNewImageState(const QString&, const QString&, const QString&, const QString&);

	void slDebugMode(bool);
	void onPassword();
	bool onReadTemperature();
	bool onReadReg();
	bool onWriteReg();
	void onTimerEvent();

	void onLoadFPACfgData();

	bool onWriteRDAC();


signals:
	void newRAWFrame(const QString&);
	void sigUpdateList();
};

#endif // SENSORCONTROL_H
