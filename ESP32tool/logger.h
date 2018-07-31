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
    /**
     * @brief class used to save files
     */
    logger();
    /**
     * @brief OpenFile
     * @param SaveFileName file which we use to save data
     */
    void OpenFile (QString SaveFileName);
    /**
     * @brief getSavingToFileFlag
     * @return true if file opened false if not opened
     */
    bool getSavingToFileFlag() const;
    ~logger();

public slots:
    /**
     * @brief stopSaving sets flag that file is closed and close port
     */
    void stopSaving();
    /**
     * @brief log logger to file time, adc1, adc2
     * @param data is vector with adc1 and adc2 data
     */
    void log(QVector<float> &data);
    void logtest(QString test);

private:
    bool savingToFileFlag; /*!< saving True, not saving(file closed) FALSE */
    QFile file;
    QTextStream stream;

};

#endif // LOGGER_H
