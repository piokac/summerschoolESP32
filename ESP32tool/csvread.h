#ifndef CSVREADER_H
#define CSVREADER_H

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <QStandardItemModel>

namespace Ui {
class CSVread;
}

class CSVread : public QDialog
{
    Q_OBJECT

public:
    explicit CSVread(QWidget *parent = 0);
    /**
     * @brief initiateWindow opens a window where we can select csv file to load and put it into table
     * @return TRUE = successful opened file, FALSE = didn't manage to open file
     */
    bool initiateWindow();
    ~CSVread();


private slots:
    void checkString(QString &temp, QChar character = 0);

private:
    Ui::CSVread *ui;
    QList<QStringList> csv;
    QStandardItemModel *model;
    QList<QStandardItem*> standardItemList;

};

#endif // CSVREAD_H
