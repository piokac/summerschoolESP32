#include "wifisettings.h"
#include "ui_wifisettings.h"
#include <esp32data.h>
#include <mainwindow.h>
wifisettings::wifisettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::wifisettings)
{
    ui->setupUi(this);
}

QString wifisettings::getSSID()
{
    return ui->SSID->text();
}

QString wifisettings::getPassword()
{
    return ui->password->text();
}

QString wifisettings::getIP()
{
    return ui->IP->text();
}

QString wifisettings::getPort()
{
    return ui->port->text();
}

wifisettings::~wifisettings()
{
    delete ui;
}

void wifisettings::setWifiSettings()
{

}
