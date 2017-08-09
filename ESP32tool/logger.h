#ifndef LOGGER_H
#define LOGGER_H
#include <QString>
#include <Qfile>
#include <QTextStream>
#include <QTime>
#include <QFileDialog>
#include <QVector>
class logger
{
public:
    logger();
    void OpenFile (QString SaveFileName);
    bool getSavingToFileFlag() const;
    ~logger();

public slots:
    void stopSaving();
    void log(QVector<int> data);

private:
    bool savingToFileFlag;
    QFile file;
    QTextStream stream;

};

#endif // LOGGER_H
