#include "esp32data.h"
#include <QtSerialPort/qserialport.h>
#include "serialportsettings.h"
#include "ui_serialportsettings.h"
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
    communication_mode=0;
    connect(serial,SIGNAL(readyRead()),this,SLOT(readPendingBytes()));
    initSocket();
    //startTimer(3000);

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

void ESP32data::closeSocket()
{
    if(udpSocket->state()>=3)
    {
        udpSocket->close();
    }
}

QString ESP32data::getPort()
{
    return serial->portName();
}


ESP32data::~ESP32data()
{
    closePort();
    closeSocket();
}

int ESP32data::getCommunication_mode() const
{
    return communication_mode;
}

void ESP32data::setCommunication_mode(int value)
{
    communication_mode = value;
}



void ESP32data::initSocket()
{
    udpSocket = new QUdpSocket(this);
    if(udpSocket->bind(QHostAddress::Any, 7755))
    {
    }
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(readPendingDatagrams()));
    tcpServer=new QTcpServer(this);
    tcpSocket=new QTcpSocket(this);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(newConnection()));
    tcpServer->listen(QHostAddress::Any, 7756);
    //tcpSocket=tcpServer->nextPendingConnection();
    //tcpSocket=tcpServer->nextPendingConnection();
    // tcpSocket->connectToHost(QHostAddress::Any, 7756);
    //connect(tcpSocket, &QAbstractSocket::disconnected,tcpSocket, &QObject::deleteLater);
    // connect(udpSocket, SIGNAL(readyRead()),
    //  this, SLOT(readPendingDatagrams()));
}


/*
void ESP32data::processTheDatagram(QNetworkDatagram datagram)
{
    QByteArray receivedData=datagram.data();
    //qDebug() << receivedData.size();

    static string frame;
    frame+=string(receivedData);
    smatch containSizeandCmd;
    smatch containAdcsData;
    regex findSizeAndCmd("\\#(\\d+)\\s(\\w)\\s([^\\*]*)\\*");
    char cmd;
    string size;
    if(frame.length()>0)
    {
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
                    qDebug() << stoi(containAdcsData[1]);
                    emit newDataAdc(stoi(containAdcsData[1]),stoi(containAdcsData[2]));
                }
                break;
            }
            }
        }
    }
}

*/






void ESP32data::processTheDatagram(QNetworkDatagram datagram)
{
    QByteArray receivedData=datagram.data();
    // qDebug() << receivedData;
    if((unsigned int)receivedData.size()>0)
    {
        if(((unsigned char)receivedData[0]=='#') && ((unsigned char)receivedData.size()==(unsigned char)receivedData[1]) &&((unsigned char)receivedData[receivedData.size()-1]=='*'))
        {
            unsigned char cmd;
            cmd=receivedData[3];
            //qDebug()<< "Accel: " <<(unsigned char)receivedData[5] << " : " << (unsigned char)receivedData[6] << " : " << (unsigned char)receivedData[7] << " : " <<  (unsigned char)receivedData[8] <<" : " << (unsigned char)receivedData[9] << " : " << (unsigned char)receivedData[10];
            //qDebug()<< "Gyro: " <<(unsigned char)receivedData[12] << " : " << (unsigned char)receivedData[13] << " : " << (unsigned char)receivedData[14] << " : " <<  (unsigned char)receivedData[15] <<" : " << (unsigned char)receivedData[16] << " : " << (unsigned char)receivedData[17];
            //qDebug()<< "Magnet: " <<(unsigned char)receivedData[19] << " : " << (unsigned char)receivedData[20] << " : " << (unsigned char)receivedData[21] << " : " <<  (unsigned char)receivedData[22] <<" : " << (unsigned char)receivedData[23] << " : " << (unsigned char)receivedData[24];
            switch(cmd)
            {
            case 'G':
            {
                QVector<short> accelData;
                QVector<short> gyroData;
                QVector<short> magnetData;
                // short debugAccel;
                //short debugGyro;
                // short debugMagnet;


                QString text;
                QTextStream str(&text);
                for(int i=0;i<3;i++)
                {
                    short val = (((unsigned short)((unsigned char)receivedData[2*i+5])) | ((unsigned short)((unsigned char)receivedData[2*i+5+1]))<<8);
                    accelData.push_back(val);
                    // debugAccel=val;
                    val = (((unsigned short)((unsigned char)receivedData[2*i+12])) | ((unsigned short)((unsigned char)receivedData[2*i+12+1]))<<8);
                    gyroData.push_back(val);
                    //debugGyro=val;
                    val = (((unsigned short)((unsigned char)receivedData[2*i+19])) | ((unsigned short)((unsigned char)receivedData[2*i+19+1]))<<8);
                    magnetData.push_back(val);
                    // debugMagnet=val;
                    // str<< "Accel: "<< debugAccel << "Gyro: "<< debugGyro << "Magnet: " << debugMagnet;

                }
                //qDebug()<<text;
                emit newDataIMU(accelData,gyroData,magnetData);
                break;
            }
            case 'A':
            {
                //qDebug()<<"in";
                int adc1data;
                int adc2data;
                string frame=string(receivedData);
                //qDebug()<< QString(frame);
                adc1data=(unsigned char)frame[5];
                adc2data=(unsigned char)frame[7];
                // qDebug() <<adc1data << " " << adc2data;
                emit newDataAdc(adc1data,adc2data);
                break;
            }
            }

        }
    }
    /*
    static string frame;
    frame+=string(receivedData);


    //emit(testData(QString::fromStdString(frame)));
    smatch containSizeandCmd;
    smatch containAdcsData;
    smatch containImuData;
    regex findSizeAndCmd("\\#(\\d+)\\s(\\w)\\s([^\\*]*)\\*");
    char cmd;
    string size;

    if(frame.length()>0)
    {
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
                //regex pattern("\\s*(\\d+)\\s+(\\d+)"); // for 2 numbers
                regex pattern("\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)"); //for 18 numbers
                if (std::regex_search(data, containImuData, pattern))
                {
                    //emit newDataAdc(stoi(containAdcsData[1]),stoi(containAdcsData[2]));
                    QVector<int> accelData{stoi(containImuData[1]),stoi(containImuData[2]),stoi(containImuData[3]),stoi(containImuData[4]),stoi(containImuData[5]),stoi(containImuData[6])};
                    QVector<int> gyroData{stoi(containImuData[7]),stoi(containImuData[8]),stoi(containImuData[9]),stoi(containImuData[10]),stoi(containImuData[11]),stoi(containImuData[12])};
                    QVector<int> magnetData{stoi(containImuData[13]),stoi(containImuData[14]),stoi(containImuData[15]),stoi(containImuData[16]),stoi(containImuData[17]),stoi(containImuData[18])};
                    emit newDataIMU(accelData,gyroData,magnetData);
                    //qDebug()<<stoi(containAdcsData[1]);
                }
                break;
            }
            }
        }
    }
*/
}

void ESP32data::processTheBytes(QByteArray receivedData)
{
    static string Byteframe;
    Byteframe+=string(receivedData);
    qDebug()<<receivedData;
    /*
    smatch containSizeandCmd;
    smatch containAdcsData;
    smatch containImuData;
    regex findSizeAndCmd("\\#(\\d+)\\s(\\w)\\s([^\\*]*)\\*");
    char cmd;
    string size;

    if(Byteframe.length()>0)
    {
        while (regex_search(Byteframe, containSizeandCmd, findSizeAndCmd))
        {
            size=containSizeandCmd[1];
            cmd=((string)containSizeandCmd[2])[0];
            string data=containSizeandCmd[3];
            Byteframe = containSizeandCmd.suffix().str();
            switch(cmd)
            {
            case 'G':
            {
                //regex pattern("\\s*(\\d+)\\s+(\\d+)");
                //  if (std::regex_search(data, containAdcsData, pattern))
                // {
                //     emit newDataAdc(stoi(containAdcsData[1]),stoi(containAdcsData[2]));
                // }
                regex pattern("\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)"); //for 18 numbers
                if (std::regex_search(data, containImuData, pattern))
                {
                    //emit newDataAdc(stoi(containAdcsData[1]),stoi(containAdcsData[2]));
                    QVector<int> accelData{stoi(containImuData[1]),stoi(containImuData[2]),stoi(containImuData[3]),stoi(containImuData[4]),stoi(containImuData[5]),stoi(containImuData[6])};
                    QVector<int> gyroData{stoi(containImuData[7]),stoi(containImuData[8]),stoi(containImuData[9]),stoi(containImuData[10]),stoi(containImuData[11]),stoi(containImuData[12])};
                    QVector<int> magnetData{stoi(containImuData[13]),stoi(containImuData[14]),stoi(containImuData[15]),stoi(containImuData[16]),stoi(containImuData[17]),stoi(containImuData[18])};
                    emit newDataIMU(accelData,gyroData,magnetData);
                    //qDebug()<<stoi(containAdcsData[1]);
                }
                break;
            }
            }
        }
    }
    */
}

void ESP32data::setWifiSettings()
{
    QString info = QString("#X w %1 %2 %3 %4*")//#LENGTH s pin1 pin2* example: #9 s 1 0*
            .arg(wificonnection.SSID).arg(wificonnection.wifipassword).arg(wificonnection.addressIP).arg(wificonnection.udpport);
    QByteArray infoByte=info.toLocal8Bit();
    writeData(infoByte);
    //qDebug()<<infoByte;
}

void ESP32data::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        processTheDatagram(datagram);
    }

}

void ESP32data::readPendingBytes()
{
    qDebug()<< "inReading" << serial->bytesAvailable();
    while(serial->bytesAvailable())
    {
        QByteArray data=serial->readAll();
        processTheBytes(data);
    }
}

void ESP32data::startCommunication()
{
        callForData();
}

void ESP32data::newConnection()
{
    tcpSocket=tcpServer->nextPendingConnection();
    //tcpSocket->waitForReadyRead();
}

void ESP32data::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    // if (socketState == QAbstractSocket::UnconnectedState && communication_mode==UDPCONNECTIONSTATUS)
    // {
    //qDebug() << "in";
    //  communication_mode=NOCONNECTIONSTATUS;
    // }
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
    //qDebug()<<tcpSocket->state();
    //qDebug()<< communication_mode;
    /*
    if(communication_mode!=NOCONNECTIONSTATUS)
    {
        //if(((communication_mode==UDPCONNECTIONSTATUS) && (udpSocket->hasPendingDatagrams())) || ((communication_mode==UARTCONNECTIONSTATUS) && (serial->canReadLine())))
        {
            //odpytywanie
            // qDebug()<<"called";
            //callForData();
            QByteArray receivedData;
            //wyslac komende
            receivedData = readData();
            //qDebug()<<receivedData;
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
    }
    */
}

void ESP32data::writeData(QByteArray data)
{
    if(communication_mode==UARTCONNECTIONSTATUS)
    {
        serial->write(data);
        qDebug()<<"dataWritten" << data;
    }
    else if(communication_mode==UDPCONNECTIONSTATUS)
    {
        //qDebug()<< tcpSocket->state();
        if(tcpSocket->waitForConnected())
        {
            qDebug() << "Written: " << data;
            tcpSocket->write(data); //write the data itself
        }
        else
        {
            qDebug() <<"Not connected";
        }

    }
}



void ESP32data::callForData()
{
    QString info = QString("#5 g*");
    QByteArray infoByte=info.toLocal8Bit();//(reinterpret_cast<const char*>(&freq), sizeof(info));
    writeData(infoByte);
}

QByteArray ESP32data::readData()
{
    QByteArray info=0;
    if(communication_mode==UARTCONNECTIONSTATUS)
    {
        info = serial->readAll();
    }
    else if(communication_mode==UDPCONNECTIONSTATUS)
    {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        info=datagram.data();
        //qDebug()<<info;
    }
    qDebug()<<info;
    return info;
}
