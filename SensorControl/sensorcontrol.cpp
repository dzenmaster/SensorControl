#define NOMINMAX

#include "sensorcontrol.h"

#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QTimer>
#include <QProcess>


#include "WaitingThread.h"

#include "PswDlg.h"

QSettings g_Settings("SensorControl","SensorControl");

extern QString g_basePath;
extern QString selfName;
extern QString GPswFileName;

int g_th = 288;
int g_tw = 384;

QString g_imodes[]=
{
	"mono16",
	"mono12",
	"mono8"
};


bool getVersionInfo(unsigned short* pwMSHW, unsigned short* pwMSLW, unsigned short* pwLSHW, unsigned short* pwLSLW)
{	
	if ((!pwMSHW)||(!pwMSLW)||(!pwLSHW)||(!pwLSLW))
		return false;
	*pwMSHW = *pwMSLW = *pwLSHW = *pwLSLW = 0;
	LPDWORD lpdwHandleToZero = 0; 
	DWORD dwSizeFVerInf = ::GetFileVersionInfoSizeW((LPCWSTR)selfName.utf16(), lpdwHandleToZero);		
	LPVOID lpFixedFileInf = new char[dwSizeFVerInf];
	BOOL bRet = ::GetFileVersionInfoW((LPCWSTR)selfName.utf16(), NULL, dwSizeFVerInf, lpFixedFileInf);
	if(bRet) {
		VS_FIXEDFILEINFO *pFixedFileInfo; 
		UINT uLen = 0;                   
		QString dsl = "\\";
		bRet = VerQueryValueW((const LPVOID)lpFixedFileInf,	L"\\", (LPVOID *) (&pFixedFileInfo), &uLen);
		if(bRet) {
			*pwMSHW = HIWORD (pFixedFileInfo->dwFileVersionMS);
			*pwMSLW = LOWORD(pFixedFileInfo->dwFileVersionMS);
			*pwLSHW = HIWORD (pFixedFileInfo->dwFileVersionLS);
			*pwLSLW = LOWORD(pFixedFileInfo->dwFileVersionLS);
		}
	}
	delete[] lpFixedFileInf;
	return bRet;
}


SensorControl::SensorControl(QWidget *parent)
	: QMainWindow(parent), m_handle(0),m_waitingThread(0), m_flashID(-1),
	m_inputLength(0),m_readBytes(0),m_inputFile(0),m_startAddr(0),m_fileType(FILE_RBF),
	m_timer4WaitFrame(0),m_frameCnt(0),m_temperature(0)
{
	ui.setupUi(this);
	
	m_timer = new QTimer;

	ui.wTerm->hide();
	ui.progressBarRBF->hide();

	//version
	unsigned short v1,v2,v3,v4;
	if (getVersionInfo(&v1,&v2,&v3,&v4))
		setWindowTitle(QString("Sensor Control ver. %1.%2.%3.%4").arg(v1).arg(v2).arg(v3).arg(v4) );

	m_framesPath = g_basePath+"Frames";
	if (!QFile::exists(m_framesPath)) {
		QDir td;
		if (!td.mkpath(m_framesPath)) {
		//	ui.teJournal->addMessage("Невозможно создать путь ",m_framesPath,1);		
		}
	}
	updateFramesList();

	setRbfFileName(g_Settings.value("rbfFileName", "").toString());
	
	bool tDebugMode = false;
	if (g_Settings.contains("debugMode")){
		tDebugMode = g_Settings.value("debugMode").toBool();
	}
	setDebugMode(tDebugMode);
	ui.cbDebugMode->setChecked(tDebugMode);

	m_buff[0]=0xA5;
	m_buff[1]=0x5A;
	memset(&m_buff[2], 0, 2046);

	connect(ui.cbShowTerminal, SIGNAL(toggled(bool)), SLOT(slShowTerminal(bool)));	
/*	connect(ui.pbSend, SIGNAL(clicked()), SLOT(slSend()));
	connect(ui.pbOpen, SIGNAL(clicked()), SLOT(slOpen()));
	connect(ui.pbClose, SIGNAL(clicked()), SLOT(slClose()));
	connect(ui.pbGetInfo, SIGNAL(clicked()), SLOT(slGetInfo()));
	connect(ui.pbClear, SIGNAL(clicked()), ui.teReceive, SLOT(clear()));
	connect(ui.pbClear, SIGNAL(clicked()),  ui.teModuleMessages, SLOT(clear()));
	connect(ui.pbBrowseRBF, SIGNAL(clicked()), SLOT(slBrowseRBF()));
	connect(ui.pbWriteFlash, SIGNAL(clicked()), SLOT(slWriteFlash()));
	connect(ui.pbReadFlashID, SIGNAL(clicked()), SLOT(slReadFlashID()));
	connect(ui.pbReadStartAddress, SIGNAL(clicked()), SLOT(slReadStartAddress()));
	connect(ui.pbEraseFlash, SIGNAL(clicked()), SLOT(slEraseFlash()));
	connect(ui.pbWriteCmdUpdateFirmware, SIGNAL(clicked()), SLOT(slWriteCmdUpdateFirmware()));
	connect(ui.pbWriteLength, SIGNAL(clicked()), SLOT(slWriteLength()));
	connect(ui.pbReadFlash, SIGNAL(clicked()), SLOT(slReadFlash()));
	connect(ui.pbUpdateFirmware, SIGNAL(clicked()), SLOT(slUpdateFirmware()));

	connect(ui.pbJumpToFact, SIGNAL(clicked()), SLOT(slJumpToFact()));
	connect(ui.pbJumpToAppl, SIGNAL(clicked()), SLOT(slJumpToAppl()));
	connect(ui.pbCancelUpdate, SIGNAL(clicked()), SLOT(slCancelUpdate()));
	connect(ui.pbClearFrameFolder, SIGNAL(clicked()), SLOT(slClearFrameFolder()));

	connect(ui.pbFCView,SIGNAL(clicked()), SLOT(slViewRaw()));
	connect(this,SIGNAL(newRAWFrame(const QString&)),SLOT(slDrawPicture(const QString&)));*/
	connect(this,SIGNAL(sigUpdateList()),SLOT(updateFramesList()));
	
/*	connect(ui.listWFrames, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),SLOT(slSelectedFrame(QListWidgetItem *,QListWidgetItem *)));
	connect(ui.pbOpenFolder, SIGNAL(clicked()), SLOT(slOpenFolder()));

	connect(ui.lView, SIGNAL(newState(const QString&, const QString&, const QString&, const QString&)), SLOT(slNewImageState(const QString&, const QString&, const QString&, const QString&)));*/
	connect(ui.cbDebugMode, SIGNAL(toggled(bool)), SLOT(slDebugMode(bool)));	
	connect(ui.pbPass,SIGNAL(clicked()),SLOT(onPassword()) );

/*	connect(ui.pbReadTemp,SIGNAL(clicked()),SLOT(onReadTemperature()) );

	connect(ui.pbReadRg,SIGNAL(clicked()),SLOT(onReadReg()) );
	connect(ui.pbWriteRg,SIGNAL(clicked()),SLOT(onWriteReg()) );
	connect(m_timer,SIGNAL(timeout()),SLOT(onTimerEvent()) );*/
	
	m_timer->start(1000);

/*	fillDeviceList();	*/

}

SensorControl::~SensorControl()
{

}

void	SensorControl::closeEvent(QCloseEvent * event)
{
	ui.wTerm->close();
}

void SensorControl::setDebugMode(bool tDM)
{
	ui.cbShowTerminal->setVisible(tDM);
	ui.gbDebug->setVisible(tDM);
}

void SensorControl::updateFramesList()
{
	ui.listWFrames->clear();
	QDir frDir(m_framesPath);
	QFileInfoList fiList = frDir.entryInfoList();
	QFileInfoList::iterator i = fiList.begin();	

	while(i!=fiList.end()) {
		QString fn = (*i).fileName();
		if ((fn!=".")&&(fn!=".."))
			ui.listWFrames->addItem((*i).fileName());
		++i;
	}
}

bool SensorControl::setRbfFileName(const QString& fn)
{
	m_fileName  = fn;
	ui.lePathToRBF->setText(m_fileName );
	if (QFile::exists(m_fileName )) {
		QFileInfo fi(m_fileName);
		ui.lSize->setText(QString("Size %1").arg(fi.size()));
		g_Settings.setValue("rbfFileName", m_fileName);
	//	ui.teJournal->addMessage("setRbfFileName", m_fileName);
		return true;
	}
	return false;
}

void SensorControl::slShowTerminal(bool st)
{
	if (st){
		ui.wTerm->setParent(0);		
		ui.wTerm->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint);
		ui.wTerm->setWindowTitle("Terminal");
		ui.wTerm->setEnabled(true);
		ui.wTerm->show();
	}
	else {
		ui.wTerm->hide();
	}
}

void SensorControl::slDebugMode(bool tDM)
{	
	if (tDM) { // check password
		QString tPswPath = g_basePath + GPswFileName;
		if (QFile::exists(tPswPath)) {
			CPswDlg tPswDlg(false,this);				
			if (tPswDlg.exec()!=QDialog::Accepted) {
				ui.cbDebugMode->setChecked(false);
				return;
			}
		}
	}
	setDebugMode(tDM);
	g_Settings.setValue("debugMode",tDM);
}

void SensorControl::onPassword()
{
	CPswDlg tPswDlg(true, this);				
	tPswDlg.exec();
}
