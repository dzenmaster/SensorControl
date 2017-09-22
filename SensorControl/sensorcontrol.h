#ifndef SENSORCONTROL_H
#define SENSORCONTROL_H

#include <QtWidgets/QMainWindow>
#include "ui_sensorcontrol.h"

class SensorControl : public QMainWindow
{
	Q_OBJECT

public:
	SensorControl(QWidget *parent = 0);
	~SensorControl();

private:
	Ui::SensorControlClass ui;
};

#endif // SENSORCONTROL_H
