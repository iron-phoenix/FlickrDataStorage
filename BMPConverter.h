#ifndef BMPCONVERTER_H
#define BMPCONVERTER_H

#include <QByteArray>
#include <QMap>
#include <QFile>
#include <QFileInfo>

class BMPConverter{
    static char const tmp[];

    static unsigned long const max_size;

    static QByteArray convertBytesToBMPBytes(QByteArray const &data);
    static QByteArray convertBMPBytesToBytes(QByteArray const &bmpdata);

public:
    static QMap<QString, QByteArray> encodeFile(QString const &inputFileName);
};

#endif // BMPCONVERTER_H
