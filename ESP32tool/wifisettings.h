#ifndef WIFISETTINGS_H
#define WIFISETTINGS_H

#include <QDialog>

namespace Ui {
class wifisettings;
}

class wifisettings : public QDialog
{
    Q_OBJECT

public:
    explicit wifisettings(QWidget *parent = 0); /*!< class for wifi settings window */
    QString getSSID();
    QString getPassword();
    QString getIP();
    QString getPort();
    ~wifisettings();

private:
    Ui::wifisettings *ui;
private slots:
    void setWifiSettings();
};

#endif // WIFISETTINGS_H
