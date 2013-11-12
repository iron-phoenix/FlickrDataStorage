#include "mainwindow.h"

#include <QDebug>
#include <QSettings>
#include <QHBoxLayout>
#include <QFile>
#include <QSplitter>
#include <QMenuBar>
#include <QMenu>
#include <QHeaderView>
#include <QFileDialog>
#include <QStatusBar>
#include <QMessageBox>

#include <QFileSystemModel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    flickrAPI = new FlickrAPI(this);
    connect(flickrAPI, SIGNAL(authResult(bool)), this, SLOT(authResult(bool)));
    connect(flickrAPI, SIGNAL(fileListLoaded(QList<FileDescription>)), this, SLOT(fileListLoaded(QList<FileDescription>)));
    connect(flickrAPI, SIGNAL(fileUploaded(QString)), this, SLOT(fileUploaded(QString)));
    connect(flickrAPI, SIGNAL(fileInfoLoaded(FileDescription)), this, SLOT(showFileInfo(FileDescription)));
    connect(flickrAPI, SIGNAL(fileDownloaded(QByteArray)), this, SLOT(fileDownloaded(QByteArray)));

    actUpload = new QAction("Upload files...", this);
    connect(actUpload, SIGNAL(triggered()), this, SLOT(uploadTriggered()));

    actLogin = new QAction("Login", this);
    connect(actLogin, SIGNAL(triggered()), this, SLOT(loginUser()));

    QMenu *fileMenu = menuBar()->addMenu("File");;
    fileMenu->addAction(actLogin);
    fileMenu->addAction(actUpload);

    cDialog = new ConnectingDialog();
    converter = new JPEGConverter(":/tmp.jpg");

    dirView = new QTreeView(this);
    QFileSystemModel *fsModel = new QFileSystemModel(this);
    fsModel->setRootPath("");
    dirView->setModel(fsModel);
    dirView->hideColumn(2);
    dirView->hideColumn(3);
    dirView->header()->setStretchLastSection(true);

    flickrFileView = new FlickrFileView(this);
    connect(flickrFileView, SIGNAL(requestDownload(FileDescription)), this, SLOT(downloadFile(FileDescription)));

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(dirView);
    splitter->addWidget(flickrFileView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    this->setCentralWidget(splitter);

    lbUserID = new QLabel(this);
    this->statusBar()->addPermanentWidget(lbUserID);
    this->statusBar()->setSizeGripEnabled(false);

    QTimer::singleShot(1000, this, SLOT(loginUser()));

}

MainWindow::~MainWindow() {
    cDialog->deleteLater();
}

void MainWindow::loginUser() {
    cDialog->setText("Logging in...");
    cDialog->show();

    QSettings appSettings("settings.ini", QSettings::IniFormat, this);
    QString token = appSettings.value("auth_token").toString();
    QString secret = appSettings.value("auth_secret").toString();
    flickrAPI->loginUser(token, secret);
}

void MainWindow::logoutUser() {
    actLogin->setText("Login");
    actLogin->disconnect(this, SLOT(logoutUser()));
    connect(actLogin, SIGNAL(triggered()), this, SLOT(loginUser()));
}

//----------------------------------------------------------------------------------

void MainWindow::uploadFile(const QString &fileName) {
    QMap<QString, QByteArray> fileParts = converter->encodeFile(fileName);
    if(fileParts.empty()) {
        qDebug() << "unable to open file";
        return;
    }
    for(QMap<QString, QByteArray>::Iterator part = fileParts.begin(); part != fileParts.end(); ++part) {
        flickrAPI->uploadFile(part.key(), part.value());
    }

}

void MainWindow::showFileInfo(FileDescription fd) {
    flickrFileView->addFile(fd);
}

//----------------------------------------------------------------------------------

void MainWindow::authResult(bool res) {
    if(res) {
        actLogin->setText("Logout");
        actLogin->disconnect(this, SLOT(loginUser()));
        connect(actLogin, SIGNAL(triggered()), this, SLOT(logoutUser()));

        lbUserID->setText(QString("Logged in as <b>%1</b>  ").arg(flickrAPI->getUsername()));

        QSettings appSettings("settings.ini", QSettings::IniFormat, this);
        appSettings.setValue("auth_token", flickrAPI->getAuthToken());
        appSettings.setValue("auth_secret", flickrAPI->getAuthSecret());
        cDialog->setText("Loading file list...");
        flickrAPI->getFileList();
    } else {
        QMessageBox::critical(this, "Error", "Authentication failed");
    }
}

//----------------------------------------------------------------------------------

void MainWindow::fileListLoaded(QList<FileDescription> files) {
    cDialog->hide();
    flickrFileView->setFileList(files);
//    this->uploadFile("D:/rescue2usb.exe");
//    flickrAPI->getFile(files.at(0));
}

void MainWindow::fileUploaded(QString id) {
    if(id.isEmpty()) {
        QMessageBox::critical(this, "Error", "Unable to uload file");
    } else {
        flickrAPI->getFileInfo(id);
    }
}

//----------------------------------------------------------------------------------

void MainWindow::downloadFile(const FileDescription &fd) {
    QString fileName = QFileDialog::getSaveFileName(this);
    if(fileName.isEmpty()) return;

    flickrAPI->getFile(fd);
}

void MainWindow::fileDownloaded(QByteArray content) {

}

//----------------------------------------------------------------------------------

void MainWindow::uploadTriggered() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select files");
    for(QStringList::Iterator fn = fileNames.begin(); fn != fileNames.end(); ++fn) uploadFile(*fn);
}

//----------------------------------------------------------------------------------

ConnectingDialog::ConnectingDialog(QWidget *parent) : QSplashScreen(parent) { /*Qt::SplashScreen | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint) {*/
    loadingIcon = new QMovie(":/loading.gif");
    loadingIcon->jumpToFrame(0);

    loadingIconTimer = new QTimer(this);
    connect(loadingIconTimer, SIGNAL(timeout()), loadingIcon, SLOT(jumpToNextFrame()));
    connect(loadingIconTimer, SIGNAL(timeout()), this, SLOT(repaint()));

    QLabel *lbIcon = new QLabel(this);
    lbIcon->setMovie(loadingIcon);

    lbText = new QLabel("some text here");

    QHBoxLayout *mlayout = new QHBoxLayout();
    mlayout->addWidget(lbIcon, 0);
    mlayout->addWidget(lbText, 1, Qt::AlignLeft);
    this->setLayout(mlayout);
}

ConnectingDialog::~ConnectingDialog() {
    loadingIconTimer->stop();
    loadingIcon->deleteLater();
}

void ConnectingDialog::setText(const QString &txt) {
    lbText->setText(txt);
}

void ConnectingDialog::showEvent(QShowEvent *) {
    loadingIcon->jumpToFrame(0);
    loadingIconTimer->start(75);
}

void ConnectingDialog::hideEvent(QHideEvent *) {
    loadingIconTimer->stop();
}
