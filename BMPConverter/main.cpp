#include <QCoreApplication>
#include <QFile>
#include <QtGui/QPixmap>
#include <QDebug>

QByteArray convertFileToByteArray(QString const &fileinput){
    QFile in(fileinput);
    QFile filebmp("/home/phoenix/tmp.bmp");

    if(!in.open(QIODevice::ReadOnly)) return 0;
    if(!filebmp.open(QIODevice::ReadOnly)) return 0;

    QByteArray ba = in.readAll();
    QByteArray tmp = filebmp.readAll();
    unsigned int size = ba.length() + 66;
    for(size_t i = 0; i != 4; ++i){
        tmp.data()[2 + i] = (char)size;
        size >>= 8;
    }
    return tmp += ba;
}

QByteArray convertBMPToByteArray(QString const &fileinput){
    QFile in(fileinput);

    if(!in.open(QIODevice::ReadOnly)) return 0;

    in.seek(66);
    QByteArray ba = in.readAll();
    return ba;
}

void uploadFiles(QByteArray data, QString inputFileName){
    if(data.length() <= 200 * 1024 * 1024){
        uploadFile(data, inputFileName + ".bmp");
    }
    else{
        size_t part_number = 1;
        QByteArray header = data.mid(0, 66);
        data = data.mid(67, -1);
        while(data.length() > (200 * 1024 * 1024) * part_number){
            QByteArray tmp = data.mid((200 * 1024 * 1024) * (part_number - 1), 200 * 1024 * 1024);
            uploadFile(tmp, inputFileName + ".part" + QString::number(part_number) + ".bmp");
            ++part_number;
        }
        QByteArray tmp = data.mid((200 * 1024 * 1024) * (part_number - 1), 200 * 1024 * 1024);
        uploadFile(tmp, inputFileName + ".part" + QString::number(part_number) + ".bmp");
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QFile savefile("/home/phoenix/1.bmp");

    if(!savefile.open(QIODevice::WriteOnly)) return 0;

    QByteArray ba = convertFileToByteArray("/home/phoenix/Downloads/Filter - Happy Together [pleer.com].mp3");

    savefile.write(ba, ba.length());


    savefile.close();

    QFile decodedFile("/home/phoenix/1.mp3");

    if(!decodedFile.open(QIODevice::WriteOnly)) return 0;

    QByteArray baE = convertBMPToByteArray("/home/phoenix/1.bmp");
    decodedFile.write(baE, baE.length());

    decodedFile.close();
    return 0;
}
