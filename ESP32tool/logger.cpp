#include "logger.h"
logger::logger()
{

}

void logger::OpenFile(QString SaveFileName)
{
    file.close();
    file.setFileName(SaveFileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        savingToFileFlag=true;
        stream.setDevice(&file);
    }
}

void logger::stopSaving()
{
    savingToFileFlag=false;
    file.close();
    // setSavingToFileFlagAndOpenFile();
    // if(state)
    // {
    // QString SaveFileName = QFileDialog::getSaveFileName(this, tr("Otwórz plik tekstowy"), tr(""), tr("All​ ​files​ ​(*.*)"));
    // OpenFile(SaveFileName);
    // }
}

void logger::log(QVector<int> data)
{
    QString format = "hh:mm:ss:zzz";
    QString str=QTime::currentTime().toString(format);
    stream.setDevice(&file);
    stream<<str;
    for(int i=0;i<data.size();i++)
    {
        stream<<", "<<data[i];
    }
    stream << "\r\n";

    stream.flush();
}

bool logger::getSavingToFileFlag() const
{
    return savingToFileFlag;
}

logger::~logger()
{
    file.close();
}
