#include <QCoreApplication>
#include <QFile>
#include <QtGui/QPixmap>
#include <QDebug>

QByteArray convertBytesToBMPBytes(QByteArray const &data){
    QFile filebmp("/home/phoenix/tmp.bmp");
    if(!filebmp.open(QIODevice::ReadOnly)) return 0;

    QByteArray tmp = filebmp.readAll();
    unsigned int size = data.length() + 66;
    for(size_t i = 0; i != 4; ++i){
        tmp.data()[2 + i] = (char)size;
        size >>= 8;
    }
    return tmp += data;
}

QByteArray convertBMPBytesToBytes(QByteArray const &bmpdata){
    return QByteArray(bmpdata.mid(66));
}

void uploadBytes(QByteArray, QString){

}

bool uploadFile(QString const &inputFileName){
    QFile in(inputFileName);
    if(!in.open(QIODevice::ReadOnly)) return false;
    QByteArray data = in.readAll();
    if(data.length() <= 200 * 1024 * 1024){
        uploadBytes(convertBytesToBMPBytes(data), inputFileName + ".bmp");
    }
    else{
        size_t part_number = 1;
        while((unsigned int)data.length() > (200 * 1024 * 1024) * part_number){
            QByteArray tmp = data.mid((200 * 1024 * 1024) * (part_number - 1), 200 * 1024 * 1024);
            uploadBytes(convertBytesToBMPBytes(tmp), inputFileName + ".part" + QString::number(part_number) + ".bmp");
            ++part_number;
        }
        QByteArray tmp = data.mid((200 * 1024 * 1024) * (part_number - 1), 200 * 1024 * 1024);
        uploadBytes(tmp, inputFileName + ".part" + QString::number(part_number) + ".bmp");
    }
    return true;
}

int main(int argc, char *argv[])
{
    QFile savefile("/home/phoenix/1.bmp");
    QFile in("/home/phoenix/Downloads/Filter - Happy Together [pleer.com].mp3");

    if(!in.open(QIODevice::ReadOnly)) return 0;
    if(!savefile.open(QIODevice::WriteOnly)) return 0;

    QByteArray inArray = in.readAll();

    QByteArray result = convertBytesToBMPBytes(inArray);

    savefile.write(result, result.length());


    in.close();
    savefile.close();

    QFile encodedFile("/home/phoenix/1.bmp");
    QFile decodedFile("/home/phoenix/1.mp3");

    if(!encodedFile.open(QIODevice::ReadOnly)) return 0;
    if(!decodedFile.open(QIODevice::WriteOnly)) return 0;

    QByteArray baE = encodedFile.readAll();
    result = convertBMPBytesToBytes(baE);
    decodedFile.write(result, result.length());

    encodedFile.close();
    decodedFile.close();
    return 0;
}
