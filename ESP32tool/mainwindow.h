#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "esp32data.h"
#include "csvread.h"
#include "logger.h"
#include <QElapsedTimer>
namespace Ui {
class MainWindow;
}
//Błędy:
//1. przy podawaniu okresu próbkowania ADC,od liczby 1000000 pojawia się notacja naukowa//POPRAWIONE
//i źle zliczana jest wtedy liczba znaków w ramce(crash programu) //POPRAWIONE
//2. ustawiając potecjometr na skrajną pozycję(wskazanie 0) a następnie ustawiając
//wartość dwuliczbową, jest duże opóźnienie
//3. program na esp32 cyklicznie wysyła dwie ramki na raz





class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void pokazKomunikacja(bool state);
    void newAdcData(int adc1,int adc2);
    void savingClicked(bool state);
    void pokazCSVreader();

private slots:

    void setFrequencyValue();
    void setPinsLevels();


private:
    Ui::MainWindow *ui;
    void updatePortStatus(bool state);
    ESP32data esp32;
    logger saverToFile;
    QElapsedTimer timer;

};

#endif // MAINWINDOW_H
