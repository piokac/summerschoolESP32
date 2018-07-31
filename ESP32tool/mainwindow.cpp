#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialportsettings.h"
#include "esp32data.h"
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <Qfile>
#include <QTextStream>
#include <QTime>
#include <QFileDialog>
#include "logger.h"
#include <wifisettings.h>
#include <QDateTime>
#include "madgwickahrs.h"
#include "visualisationwindow.h"
#include <QQuaternion>
#include <QMatrix3x3>
#include "glwidget.h"
#include "geometryengine.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    connect(ui->actionKomunikacja,SIGNAL(triggered(bool)),this,SLOT(pokazKomunikacja(bool)));
    //connect(ui->actionKomunikacja,SIGNAL(triggered(bool)),this,SLOT(updatePortStatus(bool)));
    connect(ui->setFreq,SIGNAL(clicked()),this,SLOT(setFrequencyValue()));
    connect(ui->setPins,SIGNAL(clicked()),this,SLOT(setPinsLevels()));
    connect(ui->setBias,SIGNAL(clicked()),&MadAhrs,SLOT(setRemoveBiasFalse()));
    connect(ui->setBiasVis,SIGNAL(clicked()),ui->glWidget,SLOT(setRemoveBiasFalse()));
    connect(&esp32,SIGNAL(newDataAdc(int,int)),this,SLOT(newAdcData(int,int)));
    connect(&esp32,SIGNAL(newDataIMU(QVector<short>,QVector<short>,QVector<short>)),&window,SLOT(newDataIMU(QVector<short>,QVector<short>,QVector<short>)));
    connect(&esp32,SIGNAL(newDataIMU(QVector<short>,QVector<short>,QVector<short>)),this,SLOT(newDataIMU(QVector<short>,QVector<short>,QVector<short>)));
    connect(ui->savingButton,SIGNAL(clicked(bool)),this,SLOT(savingClicked(bool)));
    connect(ui->actionZobacz_odczyty,SIGNAL(triggered()),this,SLOT(pokazCSVreader()));
    connect(ui->actionWizualizacja,SIGNAL(triggered()),this,SLOT(showVisualisationGL()));
    connect(ui->actionWiFi,SIGNAL(triggered(bool)),this,SLOT(pokazOknoWiFi(bool)));
    //connect(&esp32,SIGNAL(testData(QString)),this,SLOT(testD(QString)));
    //timer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::pokazKomunikacja(bool state)
{
    if(state)
    {
        SerialPortSettings dialog;
        if(dialog.exec() == QDialog::Accepted)
        {
            esp32.setPort(dialog.getPort());
            if(esp32.getPort()!="")
            {
                esp32.setCommunication_mode(UARTCONNECTIONSTATUS);
                esp32.startCommunication();
                ui->actionKomunikacja->setText("Rozłącz");
            }
            else
            {
                state = !state;
                esp32.setCommunication_mode(NOCONNECTIONSTATUS);
                ui->actionKomunikacja->setChecked(false);
            }
        }
        else
        {
            state = !state;
            esp32.setCommunication_mode(NOCONNECTIONSTATUS);
            ui->actionKomunikacja->setChecked(false);

        }
    }
    else
    {
        ui->actionKomunikacja->setText("Połącz przez UART");
        esp32.callForData();//to stop comming data
        esp32.setCommunication_mode(NOCONNECTIONSTATUS);
        esp32.closePort();
    }
    updatePortStatus(state);
}

void MainWindow::pokazOknoWiFi(bool state)
{
    //qDebug()<< esp32.getCommunication_mode();
    if(state)
    {
        wifisettings dialog;
        if(dialog.exec() == QDialog::Accepted)
        {
            //qDebug()<<"A";

            esp32.setCommunication_mode(UDPCONNECTIONSTATUS);
            if(esp32.getCommunication_mode()==UDPCONNECTIONSTATUS)
            {
                //qDebug()<<A;
                esp32.startCommunication();
                //timer.start();
                ui->actionWiFi->setText("Rozłącz");
            }
            else
            {
                state = !state;
                esp32.setCommunication_mode(NOCONNECTIONSTATUS);
                ui->actionWiFi->setChecked(false);
            }
        }
        else
        {
            state = !state;
            esp32.setCommunication_mode(NOCONNECTIONSTATUS);
            ui->actionWiFi->setChecked(false);

        }
    }
    else
    {
        ui->actionWiFi->setText("Połącz przez WiFi");
        esp32.callForData();//to stop comming data
        esp32.setCommunication_mode(NOCONNECTIONSTATUS);
        //esp32.closeSocket();
    }
    qDebug()<<esp32.getCommunication_mode();
    updatePortStatus(state);
}


void MainWindow::newAdcData(int adc1, int adc2)
{
    //wyświetla co 100 milisekund
    if(timer.elapsed()>100)
    {
        ui->adc1values->setValue(adc1);
        ui->adc2values->setValue(adc2);
        timer.restart();
    }
    if(saverToFile.getSavingToFileFlag()==true)
    {
        QVector<float> data;
        data.push_back((float)adc1);
        data.push_back((float)adc2);
        //saverToFile.log(data);
    }
    //if(timerSaver.elapsed()>1200000){
    //   savingClicked(false);
   // }
}

void MainWindow::newDataIMU(QVector<short> accelData, QVector<short> gyroData, QVector<short> magnetData)
{
     if(timer.elapsed()>6000)
     {
        //ui->adc1values->setValue(adc1);
         //qDebug()<<accelData[0] << " " <<accelData[1] <<" "<< accelData[2];
         float scaledGyro[3];
         for(int i=0;i<3;i++)
         {
             scaledGyro[i]=2*245.0/65535 * gyroData[i]*3.14/180;
         }
         if(MadAhrs.getRemoveBias()==false)
         {
         MadAhrs.setValZeroGyro(scaledGyro[0],scaledGyro[1],scaledGyro[2]);
         MadAhrs.setRemoveBias(true);
         //qDebug() << scaledGyro[0] << " : " << scaledGyro[1] << " : " << scaledGyro[2];
         }

         float biasTable[3]={0,0,0};
         MadAhrs.getValZeroGyro(biasTable);
         //qDebug() << scaledGyro[0] << " : " << scaledGyro[1] << " : " << scaledGyro[2];
         //qDebug()<<biasTable[0] << " : " << biasTable[1] << " : " << biasTable[2];
         //qDebug()<< "Gyro: "<< scaledGyro[0]-biasTable[0] << " : " << scaledGyro[1]-biasTable[1] << " : " << scaledGyro[2]-biasTable[2] << "  Accel: " << accelData[0] << " : " <<accelData[1] << " : " <<accelData[2] << " Magnet: " <<magnetData[0] << " : " << magnetData[1] << " : " << magnetData[2];
         //qDebug()<< gyroData[0]<< " : " << gyroData[1] << " : " << gyroData[2];
         //qDebug() << biasTable[0] << " : " << biasTable[1] << " : " << biasTable[2];
         //MadAhrs.MadgwickAHRSupdate(scaledGyro[0]-biasTable[0],scaledGyro[1]-biasTable[1],scaledGyro[2]-biasTable[2],accelData[0],accelData[1],accelData[2],magnetData[0],magnetData[1],magnetData[2]);
         QMatrix4x4 UMatrix4x4(MadAhrs.getU());
         QVector3D cVector=MadAhrs.getC();
         QVector3D magnetVector(-magnetData[0],magnetData[1],-magnetData[2]);
         QVector3D calibratedMagnet=UMatrix4x4*(magnetVector-cVector);
         //qDebug()<< magnetVector << " callibrated: " << calibratedMagnet << " : " << magnetVector+cVector;
         MadAhrs.MadgwickAHRSupdate(-(scaledGyro[0]-biasTable[0]),(scaledGyro[1]-biasTable[1]),(scaledGyro[2]-biasTable[2]),-accelData[0],accelData[1],accelData[2],-calibratedMagnet[0],calibratedMagnet[1],-calibratedMagnet[2]);
         //MadAhrs.MadgwickAHRSupdate(scaledGyro[0]-biasTable[0],scaledGyro[1]-biasTable[1],scaledGyro[2]-biasTable[2],scaledAccel[0],scaledAccel[1],scaledAccel[2],-scaledMagnet[1],-scaledMagnet[0],scaledMagnet[2]);
         //MadAhrs.MadgwickAHRSupdateIMU((float)gyroData[0],(float)gyroData[1],(float)gyroData[2],(float)accelData[0],(float)accelData[1],(float)accelData[2]);
         //MadAhrs.MadgwickAHRSupdate((float)gyroData[1], (float)gyroData[0],-(float)gyroData[2],
               //  -(float)accelData[1], (float)accelData[0], (float)accelData[2],
                // (float)magnetData[1], -(float)magnetData[0], -(float)magnetData[2]);
          // qDebug()<< -magnetData[0]<< " : "<< magnetData[1]<< " : " <<-magnetData[2];

         //QVector<float> RotationMatrix= MadAhrs.toRotMatrix();


         QVector<float> q=MadAhrs.getQ();
         ui->glWidget->updateOrientation(q[0], q[1], q[2], q[3]);
         QQuaternion quat(q[0], q[1], q[2], q[3]);
         if(ui->glWidget->getRemoveBias()==false)
         {
             ui->glWidget->setBias(quat);
             ui->glWidget->setRemoveBias(true);
         }
         //QVector3D EulerAngles=quat.toEulerAngles();
         QVector3D EulerAngles;
         quat.getEulerAngles(&EulerAngles[0],&EulerAngles[1],&EulerAngles[2]);
         QVector3D EulerAnglesTest=quat.toEulerAngles();
         if(saverToFile.getSavingToFileFlag()==true)
         {
             QVector<float> data;

             //data.push_back(-magnetData[0]);
             //data.push_back(magnetData[1]);
             //data.push_back(-magnetData[2]);
             data.push_back(-calibratedMagnet[0]);
             data.push_back(calibratedMagnet[1]);
             data.push_back(-calibratedMagnet[2]);
             data.push_back(-accelData[0]);
             data.push_back(accelData[1]);
             data.push_back(accelData[2]);
             data.push_back(-(scaledGyro[0]-biasTable[0]));
             data.push_back((scaledGyro[1]-biasTable[1]));
             data.push_back((scaledGyro[2]-biasTable[2]));
             data.push_back(EulerAngles[0]);
             data.push_back(EulerAngles[1]);
             data.push_back(EulerAngles[2]);
             saverToFile.log(data);
         }
         qDebug() << "Pitch:" << EulerAngles[0] << " Yaw: "<< EulerAngles[1] << " Roll: " << EulerAngles[2] << " Gyro: " <<-(scaledGyro[0]-biasTable[0])<< " : " << -(scaledGyro[1]-biasTable[1]) << " : " << -(scaledGyro[2]-biasTable[2]);
         qDebug() << "Pitch:" << EulerAngles[0] << " Yaw: "<< EulerAngles[1] << " Roll: " << EulerAngles[2] << " New: " << EulerAnglesTest[0] << " : " << EulerAnglesTest[1] << " : " << EulerAnglesTest[2];
         //for(int i=0;i<RotationMatrix.size();i++)
         //{
         //    qDebug() << "Rot:" << i << " : " << RotationMatrix[i];
        // }
       // qDebug()<<accelData[0] << " " <<accelData[1] <<" "<< accelData[2];
        //qDebug()<<magnetData[3];
        //ui->adc2values->setValue(adc2);
        //timer.restart();
     }
}




void MainWindow::testD(QString received)
{
    if(saverToFile.getSavingToFileFlag()==true)
    {
        saverToFile.logtest(received);
    }
}


void MainWindow::savingClicked(bool state)
{
    if(esp32.getCommunication_mode()!=NOCONNECTIONSTATUS)
    {
        if(state)
        {
            //QString SaveFileName = QFileDialog::getSaveFileName(this, tr("Otwórz plik tekstowy"), tr(""), tr("All​ ​files​ ​(*.*)"));
            //QString filename= "out.txt";
            QString filename = "C:\\out"+QDateTime::currentDateTime().toString("dd-MM-yyyyThhmmss")+".csv";
            saverToFile.OpenFile(filename);
            ui->savingButton->setText("Przerwij zapis");
            //timerSaver.start();
        }
        else
        {
            ui->savingButton->setText("Zapisuj odczyty do pliku");
            saverToFile.stopSaving();
        }
    }
}


void MainWindow::pokazCSVreader()
{
    CSVread reader;
    if(reader.initiateWindow())
    {
        if(reader.exec() == QDialog::Accepted)
        {

        }
    }
}

void MainWindow::showVisualisationGL()
{
    QSurfaceFormat format;
    format.setSamples(16);
    window.setFormat(format);
    window.resize(640, 480);
    window.show();

    window.setAnimating(true);


}

void MainWindow::pokazOknoWiz()
{

}


void MainWindow::updatePortStatus(bool state)
{
    if(state)
    {
        if(esp32.getCommunication_mode()==UARTCONNECTIONSTATUS)
        {
            ui->statusLabel->setText(tr("Status: Połączono z portem %1.")
                                     .arg(esp32.getPort()));
        }
        else if(esp32.getCommunication_mode()==UDPCONNECTIONSTATUS)
        {
            ui->statusLabel->setText(tr("Status: Połączono z wifi."));
        }
        else if(esp32.getCommunication_mode()==NOCONNECTIONSTATUS)
        {
            ui->statusLabel->setText(tr("Status: Brak połączenia."));
        }
    }
    else
    {
        if(esp32.getCommunication_mode()==NOCONNECTIONSTATUS)
        {
            ui->statusLabel->setText(tr("Status: Brak połączenia."));
        }
    }

}

void MainWindow::setFrequencyValue()
{
    QString frequency=ui->frequencyEdit->text();
    int frequencyINT=frequency.toInt();
    if(frequencyINT>0)
    {
        esp32.setFrequencyAdc(frequencyINT);
        //qDebug()<<frequencyINT;
    }
}



void MainWindow::setPinsLevels()
{
    QString pin1Level=ui->pin1Level->text();
    int pin1LevelINT=pin1Level.toInt();
    QString pin2Level=ui->pin2Level->text();
    int pin2LevelINT=pin2Level.toInt();
    if((pin1LevelINT==0 ||pin1LevelINT==1) && (pin2LevelINT==0 || pin2LevelINT==1))
    {
        esp32.setGpios(pin1LevelINT,pin2LevelINT);
    }
}

