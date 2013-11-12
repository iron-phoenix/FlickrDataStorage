#include <JPEGConverter.h>

JPEGConverter::JPEGConverter(QString const &filename): max_size(150 * 1024 * 1024){
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

void JPEGConverter::encodeFile(QString const &inputFileName, FileToUpload &fileToUpload){
    QFile in(inputFileName);
    if("" == fileToUpload.sourceFileName){
        QFileInfo fi(in.fileName());
        fileToUpload.sourceFileName = fi.fileName();
    }
    if(!in.open(QIODevice::ReadOnly)) return;
    in.seek(fileToUpload.offset);
    fileToUpload.byteArray = convertBytesToJPEGBytes(in.read(max_size));
    ++fileToUpload.partNumber;

    if(!in.atEnd()) fileToUpload.offset += max_size;
    else fileToUpload.offset = -1;
}

bool JPEGConverter::decodeFile(const QString &filename, const QByteArray &array){
    QFile in(filename);
    if(!in.open(QIODevice::Append)) return false;
    in.write(convertJPEGBytesToBytes(array));
    return true;
}
