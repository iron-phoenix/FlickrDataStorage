#ifndef BMPCONVERTER_H
#define BMPCONVERTER_H

#include <QByteArray>
#include <QMap>
#include <QFile>
#include <QFileInfo>

struct FileToUpload{
    QByteArray byteArray;
    qint32 partNumber;
    QString sourceFileName;

    explicit FileToUpload(): partNumber(0), sourceFileName(""){}

    QString getFileName(){
        return sourceFileName + ".part" + QString::number(partNumber) + ".jpeg";
    }
};

class JPEGConverter{
    QByteArray tmp;

    unsigned long const max_size;

    QByteArray convertBytesToJPEGBytes(QByteArray const &data);
    QByteArray convertJPEGBytesToBytes(QByteArray const &bmpdata);

public:
    JPEGConverter(QString const &filetmp);
    qint32 encodeFile(QString const &inputFileName, FileToUpload &fileToUpload, qint32 offset);
    bool decodeFile(QString const &filename, QByteArray const &array);
};

#endif // BMPCONVERTER_H
