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

class ESP32data : public QObject
{
    Q_OBJECT

public:
    explicit ESP32data(QObject *parent = 0);
    void closePort();
    void setPort(QString NamePort);
    QString getPort();
    ~ESP32data();

private:
    QSerialPort *serial;
    int pin1level,pin2level;
    double frequency;

signals:
    void newDataAdc(int, int);
public slots:
    void setGpios(int pin1,int pin2);
    void setFrequencyAdc(double freq);
    void timerEvent(QTimerEvent* ev);
    void writeData(QByteArray data);
    void callForData();
    QByteArray readData();
    //void handleTimeout();

};

#endif // ESP32DATA_H