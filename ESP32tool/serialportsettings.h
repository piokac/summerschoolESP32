#ifndef SERIALPORTSETTINGS_H
#define SERIALPORTSETTINGS_H

#include <QDialog>
#include "esp32data.h"

namespace Ui {
class SerialPortSettings;
}

class SerialPortSettings : public QDialog
{
    Q_OBJECT

public:
    explicit SerialPortSettings(QWidget *parent = 0); /*!< class for serial communication window*/
    ~SerialPortSettings();
    QString getPort();

    QString currentPort;

private slots:

    void showConnectionStatus();

    void saveSettings();

    //void on_buttonBox_accepted();

private:
    Ui::SerialPortSettings *ui;
};

#endif // SERIALPORTSETTINGS_H
