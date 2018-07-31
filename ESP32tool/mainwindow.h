#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "esp32data.h"
#include "csvread.h"
#include "logger.h"
#include "madgwickahrs.h"
#include "visualisationwindow.h"
#include <QElapsedTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void pokazKomunikacja(bool state);
    void pokazOknoWiFi(bool state);
    /**
     * @brief newAdcData slot to receive data from emited signal, process adc values
     * @param adc1 value adc1
     * @param adc2 value adc2
     */
    void newAdcData(int adc1,int adc2);
    void newDataIMU(QVector<short> accelData, QVector<short> gyroData, QVector<short> magnetData);
    void testD(QString received);
    void savingClicked(bool state);
    void pokazCSVreader();
    void showVisualisationGL();
    void pokazOknoWiz();

private slots:

    void setFrequencyValue();
    void setPinsLevels();


private:
    Ui::MainWindow *ui;
    void updatePortStatus(bool state);
    ESP32data esp32;
    visualisationWindow window;

    logger saverToFile;
    QElapsedTimer timer;
    MadgwickAHRS MadAhrs;
    //QElapsedTimer timerSaver;

};

#endif // MAINWINDOW_H
