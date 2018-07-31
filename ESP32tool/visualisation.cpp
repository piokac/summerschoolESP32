#include "visualisation.h"
#include "ui_visualisation.h"

visualisation::visualisation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::visualisation)
{
    ui->setupUi(this);
}

visualisation::~visualisation()
{
    delete ui;
}
