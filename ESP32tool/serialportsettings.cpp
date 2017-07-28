#include "serialportsettings.h"
#include "ui_serialportsettings.h"
#include "esp32data.h"
#include "mainwindow.h"
#include "serialportsettings.h"
#include "QtSerialPort/QSerialPort"
#include "QtSerialPort"
#include "QSerialPort"
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>

SerialPortSettings::SerialPortSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialPortSettings)
{
    ui->setupUi(this);
    //const auto infos = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->serialPortComboBox->addItem(info.portName());
    showConnectionStatus();
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(saveSettings()));
    //connect(ui->buttonBox,SIGNAL(clicked()),this,SLOT(showConnectionStatus()));
    //connect(ui->serialPortComboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(showConnectionStatus()));
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(accept()));
    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(reject()));
}


SerialPortSettings::~SerialPortSettings()
{
    //serial.closePort();
    delete ui;
}

void SerialPortSettings::showConnectionStatus()
{
    //ui->statusLabel->setText(tr("Status: Połączono z portem %1.")
                            // .arg(ui->serialPortComboBox->currentText()));
}

QString SerialPortSettings::getPort()
{
    QString port=currentPort;
    return port;
}

void SerialPortSettings::saveSettings()
{
    currentPort=ui->serialPortComboBox->currentText();
    //qDebug()<<currentPort;
}
