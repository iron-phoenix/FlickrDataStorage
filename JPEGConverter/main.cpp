#include <QCoreApplication>
#include <QFile>
#include <QtGui/QPixmap>
#include <QDebug>
#include <JPEGConverter.h>

int main(int argc, char *argv[])
{
    JPEGConverter jpg("/home/phoenix/tmp.jpg");
    QMap<QString, QByteArray> result = jpg.encodeFile("/home/phoenix/Downloads/setup-x86.exe");

    for(QMap<QString, QByteArray>::Iterator it = result.begin(); it != result.end(); ++it){
        QFile savefile(it.key());
        if(!savefile.open(QIODevice::WriteOnly)) return 0;
        savefile.write(it.value(), it.value().length());
        savefile.close();
    }

//    QFile encodedFile("/home/phoenix/1.bmp");
//    QFile decodedFile("/home/phoenix/1.mp3");l

//    if(!encodedFile.open(QIODevice::ReadOnly)) return 0;
//    if(!decodedFile.open(QIODevice::WriteOnly)) return 0;

//    QByteArray baE = encodedFile.readAll();
//    result = convertBMPBytesToBytes(baE);
//    decodedFile.write(result, result.length());

//    encodedFile.close();
//    decodedFile.close();
    return 0;
}
