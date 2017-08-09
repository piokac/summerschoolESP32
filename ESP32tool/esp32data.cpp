#include "esp32data.h"
#include <QtSerialPort/qserialport.h>
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
#include <string>
#include <regex>
#include <math.h>

QT_USE_NAMESPACE
using namespace std;

ESP32data::ESP32data(QObject *parent) : QObject(parent),serial(new QSerialPort(this))
{
    serial->setPortName("");
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    startTimer(100);
}

void ESP32data::closePort()
{
    if(serial->isOpen())
    {
        serial->close();
    }
}

void ESP32data::setPort(QString NamePort)
{
    serial->setPortName(NamePort);
    closePort();
    serial->open(QIODevice::ReadWrite);

}

QString ESP32data::getPort()
{
    return serial->portName();
}


ESP32data::~ESP32data()
{
    closePort();
}


void ESP32data::setGpios(int pin1, int pin2)
{
    this->pin1level=pin1;
    this->pin2level=pin2;
    if(pin1>=1)
    {
        this->pin1level=1;
    }
    if(pin2>=1)
    {
        this->pin2level=1;
    }

    //due to this ifs dataLength will be always equal to 9
    //int dataLength=9;//should be always 9
    QString info = QString("#9 s %1 %2*")//#LENGTH s pin1 pin2* example: #9 s 1 0*
            .arg(pin1level).arg(pin2level);
    QByteArray infoByte=info.toLocal8Bit();
    //const char *str;
    //str=infoByte.data();
    writeData(infoByte);
    //qDebug()<<info;
    //qDebug()<<this->serialPort;

}

void ESP32data::setFrequencyAdc(double freq)
{

    this->frequency=freq;
    QString freqString=QVariant(freq).toString();
    QString frameData= QString(" f %1*")
            .arg(freqString);
    double frameDataLength=frameData.length()+1;

    double LogOfFrameDataLength= log((double)frameDataLength+1.0)/log(10.0); // it equals log of base of 10th of frameDataLength
    double frameLengthNumberOfDigits = 0;
    frameLengthNumberOfDigits = floor(LogOfFrameDataLength) + 1.0;
    int frameLength=0;
    frameLength = frameDataLength + frameLengthNumberOfDigits;

    QString frame= QString("#%1 f %2*")
            .arg(frameLength).arg(freqString);
    QByteArray frameByte=frame.toLocal8Bit();
    writeData(frameByte);
    //qDebug()<<frameByte;
    //    //int dataLength=1+1+1+1+freqString.length()+1;//number of signs in package without LENGTH number of signs
    //    //QString dataLengthString=QString::number(dataLength);;//number of digits of dataLength
    //    //to get complete number of signs(LENGHT),add number of digits of dataLenght and number of signs in package withoug LENGHT
    //    //example: #LENGHT f 100*  dataLenght=number of signs without LENGTH number of signs + number of signs of
    //    //                         dataLenght=X+Y; // X=number of signs without LENGTH number of signs, Y=number of signs of X
    //    //from example: dataLength=8+1=9;
    //    if(dataLength==9)
    //    {
    //        dataLength=dataLength+dataLengthString.length()+1;//dataLength=9+1+1
    //    }
    //    else if((dataLength==98) || (dataLength==99))
    //    {
    //        dataLength=dataLength+dataLengthString.length()+1;//dataLength=99+2+1
    //    }
    //    else
    //    {
    //        dataLength=dataLength+dataLengthString.length();
    //    }
    //    //else if(((dataLenght+dataLengthString.lenght()-(dataLenghtString.length()-()))%10==0)//if dataLength would be 99 we need to add 3 instead of 2
    //    //now dataLength is the same like LENGTH

    //    QString info = QString("#%1 f %2*")//#LENGTH f freq* example: #7 f 1*
    //            .arg(dataLength).arg(freq);
    //    QByteArray infoByte=info.toLocal8Bit();//(reinterpret_cast<const char*>(&freq), sizeof(info));
    //    //const char *str;
    //    //str=infoByte.data();
    //    writeData(infoByte);
    //    //qDebug()<<info[i];

}


void ESP32data::timerEvent(QTimerEvent *ev)
{
    //odpytywanie
    // qDebug()<<"called";
    callForData();
    //wyslac komende
    QByteArray receivedData = readData();
    //odbierz ramke
    static string frame;
    frame+=string(receivedData);
    //qDebug()<<receivedData;
    smatch containSizeandCmd;
    smatch containAdcsData;
    regex findSizeAndCmd("\\#(\\d+)\\s(\\w)\\s([^\\*]*)\\*");
    char cmd;
    string size;
    if(frame.length()>0)
    {
        //qDebug()<<frame.length();
        while (regex_search(frame, containSizeandCmd, findSizeAndCmd))
        {
            size=containSizeandCmd[1];
            cmd=((string)containSizeandCmd[2])[0];
            string data=containSizeandCmd[3];

            frame = containSizeandCmd.suffix().str();


            switch(cmd)
            {
            case 'G':
            {
                regex pattern("\\s*(\\d+)\\s+(\\d+)");
                if (std::regex_search(data, containAdcsData, pattern))
                {
                    emit newDataAdc(stoi(containAdcsData[1]),stoi(containAdcsData[2]));
                }
                break;
            }
            }
        }
       // if(frame.length()>0)
       // {
        //    qDebug()<<QString::fromStdString(frame);
        //}
    }
    //emitować i slot wyświetlający w mainwindow wartości
}

void ESP32data::writeData(QByteArray data)
{
    serial->write(data);
}

void ESP32data::callForData()
{
    QString info = QString("#5 g*");
    QByteArray infoByte=info.toLocal8Bit();//(reinterpret_cast<const char*>(&freq), sizeof(info));
    writeData(infoByte);

}

QByteArray ESP32data::readData()//TODO
{
    QByteArray info = serial->readAll();
    //qDebug()<<info;
    return info;
}









