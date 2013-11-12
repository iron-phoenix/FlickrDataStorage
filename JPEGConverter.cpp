#include <JPEGConverter.h>

//char const JPEGConverter::tmp[] = {0x42, 0x4d, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00,
//                           0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
//                           0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//                           0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x80, 0x00, 0x00, 0x00};

JPEGConverter::JPEGConverter(QString const &filename): max_size(200 * 1024 * 1024){
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    tmp = file.readAll();
}

QByteArray JPEGConverter::convertBytesToJPEGBytes(QByteArray const &data){
    QByteArray result(tmp);
    result += data;
    return result;
}

QByteArray JPEGConverter::convertJPEGBytesToBytes(QByteArray const &jpegdata){
    return QByteArray(jpegdata.mid(631));
}

QMap<QString, QByteArray> JPEGConverter::encodeFile(QString const &inputFileName){
    QFile in(inputFileName);
    QFileInfo fi(in.fileName());
    QString newFileName(fi.fileName());
    QMap<QString, QByteArray> result;
    if(!in.open(QIODevice::ReadOnly)) return result;
    QByteArray data = in.readAll();
    if((unsigned int)data.length() <= max_size){
        result[newFileName + ".jpeg"] = convertBytesToJPEGBytes(data);
    }
    else{
        size_t part_number = 1;
        while((unsigned int)data.length() > max_size * part_number){
            result[newFileName + ".part" + QString::number(part_number) + ".jpeg"] = convertBytesToJPEGBytes(data.mid(max_size * (part_number - 1), max_size));
            ++part_number;
        }
        result[newFileName + ".part" + QString::number(part_number) + ".jpeg"] = convertBytesToJPEGBytes(data.mid(max_size * (part_number - 1), max_size));
    }
    return result;
}
