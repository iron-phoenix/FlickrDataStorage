#ifndef BMPCONVERTER_H
#define BMPCONVERTER_H

#include <QByteArray>
#include <QMap>
#include <QFile>
#include <QFileInfo>

class JPEGConverter{
    QByteArray tmp;

    unsigned long const max_size;

    QByteArray convertBytesToJPEGBytes(QByteArray const &data);
    QByteArray convertJPEGBytesToBytes(QByteArray const &bmpdata);

public:
    JPEGConverter(QString const &filetmp);
    QMap<QString, QByteArray> encodeFile(QString const &inputFileName);
};

#endif // BMPCONVERTER_H
