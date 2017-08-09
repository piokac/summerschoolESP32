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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionKomunikacja,SIGNAL(triggered(bool)),this,SLOT(pokazKomunikacja(bool)));
    //connect(ui->actionKomunikacja,SIGNAL(triggered(bool)),this,SLOT(updatePortStatus(bool)));
    connect(ui->setFreq,SIGNAL(clicked()),this,SLOT(setFrequencyValue()));
    connect(ui->setPins,SIGNAL(clicked()),this,SLOT(setPinsLevels()));
    connect(&esp32,SIGNAL(newDataAdc(int,int)),this,SLOT(newAdcData(int,int)));
    connect(ui->savingButton,SIGNAL(clicked(bool)),this,SLOT(savingClicked(bool)));
    connect(ui->actionZobacz_odczyty,SIGNAL(triggered()),this,SLOT(pokazCSVreader()));
    timer.start();
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
                ui->actionKomunikacja->setText("Rozłącz");
            }
            else
            {
                state = !state;
                ui->actionKomunikacja->setChecked(false);
            }

        }
        else
        {
            state = !state;
            ui->actionKomunikacja->setChecked(false);
        }
    }
    else
    {
        ui->actionKomunikacja->setText("Połącz");
        esp32.closePort();
    }
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
        QVector<int> data;
        data.push_back(adc1);
        data.push_back(adc2);
        saverToFile.log(data);
    }
}

void MainWindow::savingClicked(bool state)
{
    if(esp32.getPort()!="")
    {
        if(state)
        {
            QString SaveFileName = QFileDialog::getSaveFileName(this, tr("Otwórz plik tekstowy"), tr(""), tr("All​ ​files​ ​(*.*)"));
            saverToFile.OpenFile(SaveFileName);
            ui->savingButton->setText("Przerwij zapis");
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


void MainWindow::updatePortStatus(bool state)
{
    if(state)
    {
        if(esp32.getPort()!= "")
        {
            ui->statusLabel->setText(tr("Status: Połączono z portem %1.")
                                     .arg(esp32.getPort()));
        }
        else
        {
            ui->statusLabel->setText(tr("Status: Brak połączenia.")
                                     .arg(esp32.getPort()));
        }
    }
    else
    {
        if(esp32.getPort()!= "")
        {
            ui->statusLabel->setText(tr("Status: Rozłączono.")
                                     .arg(esp32.getPort()));
        }
        else
        {
            ui->statusLabel->setText(tr("Status: Brak połączenia.")
                                     .arg(esp32.getPort()));
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

