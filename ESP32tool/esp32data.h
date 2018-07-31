#ifndef ESP32DATA_H
#define ESP32DATA_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "serialportsettings.h"
#include "QTimer"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>



#define COMMUNICATIONINTERVAL 100
#define NOCONNECTIONSTATUS 0
#define UARTCONNECTIONSTATUS 1
#define UDPCONNECTIONSTATUS 2

class ESP32data : public QObject
{
    Q_OBJECT


public:
    /**
     * @brief ESP32data class is used to communication with ESP module, it doesn't create any windows,it just class for communication
     *
     */
    explicit ESP32data(QObject *parent = 0);
    /**
     * @brief closePort closing uart communication port
     */
    void closePort();
    /**
     * @brief setPort selects port in computer where ESP32 is connected, for example COM3
     * @param NamePort port, example COM3
     */
    void setPort(QString NamePort);
    /**
     * @brief closeSocket closing UDP socket
     */
    void closeSocket();
    QString getPort();
    /**
     * @brief initSocket opens a socket on a computer, in summerschool computer was a server and ESP was a client
     */
    void initSocket();
    /**
     * @brief setWifiSettings it loads values from window and sends it to ESP32 in frame, TODO: WifiSettings struct is public- change it
     */
    void setWifiSettings();
    struct WifiSettings
    {
        QString SSID;
        QString wifipassword;
        QString addressIP;
        QString udpport;
    };
    WifiSettings wificonnection; /*!< struct object which contais wifi settings*/
    ~ESP32data();

    int getCommunication_mode() const;
    void setCommunication_mode(int value);

private:
    QSerialPort *serial;
    QUdpSocket *udpSocket;
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;

    int communication_mode;//0-no connection, 1-uart, 2-wifi

    int pin1level,pin2level; /*!< pin1level and pin2level which we want to set on ESP*/
    double frequency;/*!< frequency which we want to set on ESP*/
    /**
     * @brief processTheDatagram managing data from udp
     * @param datagram datagram is a packet which we get in udp communication
     */
    void processTheDatagram(QNetworkDatagram datagram);
    void processTheBytes(QByteArray receivedData);


signals:
    /**
     * @brief newDataAdc we get data and emit a signal- we send adc1 and adc2 values in arguments(QT automaticy get it it when we connect with slot) after connecting data go to slot and we can process this data in the slot
     */
    void newDataAdc(int, int);

    void newDataIMU(QVector<short>,QVector<short>,QVector<short>);
    void testData(QString);
public slots:
    /**
     * @brief setGpios set pins levels and send frame
     * @param pin1 level
     * @param pin2 level
     */
    void setGpios(int pin1,int pin2);
    /**
     * @brief setFrequencyAdc and send frame
     * @param freq
     */
    void setFrequencyAdc(double freq);
    /**
     * @brief timerEvent timer which contains getting data from ESP32
     * @param ev
     */
    void timerEvent(QTimerEvent* ev);
    /**
     * @brief writeData send data on UART
     * @param data QByteArray data
     */
    void writeData(QByteArray data);
    /**
     * @brief callForData sends a frame and then ESP32 answers adcs values in frame
     */
    void callForData();
    /**
     * @brief readData read data from uart
     * @return
     */
    QByteArray readData();
    /**
     * @brief readPendingDatagrams receive datagrams in UDP
     */
    void readPendingDatagrams();
    void readPendingBytes();
    void startCommunication();
    void newConnection();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
};

#endif // ESP32DATA_H
