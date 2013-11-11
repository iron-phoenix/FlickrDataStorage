#include "mainwindow.h"
#include "BMPConverter.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    flickrAPI = new FlickrAPI(this);
    connect(flickrAPI, SIGNAL(authResult(bool)), this, SLOT(authResult(bool)));
    connect(flickrAPI, SIGNAL(fileListLoaded(QList<FileDescription>)), this, SLOT(fileListLoaded(QList<FileDescription>)));
    connect(flickrAPI, SIGNAL(fileUploaded(QString)), this, SLOT(fileUploaded(QString)));

    flickrAPI->loginUser("72157637554344916-563cd139c3125c4c", "5ef4535767460ae0");
}

MainWindow::~MainWindow() {
}

//----------------------------------------------------------------------------------

void MainWindow::uploadFile(const QString &fileName) {
    QMap<QString, QByteArray> fileParts = BMPConverter::encodeFile(fileName);
    if(fileParts.empty()) {
        qDebug() << "unable to open file";
        return;
    }
    for(QMap<QString, QByteArray>::Iterator part = fileParts.begin(); part != fileParts.end(); ++part) {
        QFile out(part.key());
        out.open(QFile::WriteOnly);
        out.write(part.value());
        out.flush();
        out.close();
        flickrAPI->uploadFile("123", part.value());
    }

}

void MainWindow::authResult(bool res) {
    if(res) {
        qDebug() << "Auth succeed";
        uploadFile("D:/setup-x86.exe");
//        flickrAPI->getFileList();
    } else {
        qDebug() << "Auth failed";
    }
}

void MainWindow::fileListLoaded(QList<FileDescription> files) {
    flickrAPI->getFile(files.at(0));
}

void MainWindow::fileUploaded(QString id) {

}
