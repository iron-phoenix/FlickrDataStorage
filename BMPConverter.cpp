#include <BMPConverter.h>

char const BMPConverter::tmp[] = {0x42, 0x4d, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00,
                           0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
                           0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x80, 0x00, 0x00, 0x00};

unsigned long const BMPConverter::max_size = 200 * 1024 * 1024;

QByteArray BMPConverter::convertBytesToBMPBytes(QByteArray const &data){
    QByteArray result(tmp, sizeof(tmp));
    result += data;
    unsigned int size = result.length();
    for(size_t i = 0; i != 4; ++i){
        result.data()[2 + i] = (unsigned char)size;
        size >>= 8;
    }
    return result;
}

QByteArray BMPConverter::convertBMPBytesToBytes(QByteArray const &bmpdata){
    return QByteArray(bmpdata.mid(66));
}

QMap<QString, QByteArray> BMPConverter::encodeFile(QString const &inputFileName){
    QFile in(inputFileName);
    QFileInfo fi(in.fileName());
    QString newFileName(fi.fileName());
    QMap<QString, QByteArray> result;
    if(!in.open(QIODevice::ReadOnly)) return result;
    QByteArray data = in.readAll();
    if((unsigned int)data.length() <= max_size){
        result[newFileName + ".bmp"] = convertBytesToBMPBytes(data);
    }
    else{
        size_t part_number = 1;
        while((unsigned int)data.length() > max_size * part_number){
            result[newFileName + ".part" + QString::number(part_number) + ".bmp"] = convertBytesToBMPBytes(data.mid(max_size * (part_number - 1), max_size));
            ++part_number;
        }
        result[newFileName + ".part" + QString::number(part_number) + ".bmp"] = convertBytesToBMPBytes(data.mid(max_size * (part_number - 1), max_size));
    }
    return result;
}
