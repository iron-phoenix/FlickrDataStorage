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
    qint64 offset;

    explicit FileToUpload(): partNumber(0), sourceFileName(""), offset(0) {}

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
    void encodeFile(QString const &inputFileName, FileToUpload &fileToUpload);
    bool decodeFile(QString const &filename, QByteArray const &array);
};

#endif // BMPCONVERTER_H
