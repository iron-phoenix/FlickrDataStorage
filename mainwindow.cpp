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
#include <QIcon>
#include <QPixmap>

#include <QFileSystemModel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    flickrAPI = new FlickrAPI(this);
    connect(flickrAPI, SIGNAL(authResult(bool)), this, SLOT(authResult(bool)));
    connect(flickrAPI, SIGNAL(fileListLoaded(QList<BigFileDescription>)), this, SLOT(fileListLoaded(QList<BigFileDescription>)));
    connect(flickrAPI, SIGNAL(fileUploaded(FileDescription, QString)), this, SLOT(fileUploaded(FileDescription, QString)));
//    connect(flickrAPI, SIGNAL(fileInfoLoaded(FileDescription)), this, SLOT(showFileInfo(FileDescription)));
    connect(flickrAPI, SIGNAL(fileDownloaded(QByteArray,QString)), this, SLOT(fileDownloaded(QByteArray,QString)));

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
    connect(flickrFileView, SIGNAL(requestDownload(BigFileDescription)), this, SLOT(downloadFile(BigFileDescription)));

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
    getFileIcon(fileName);  //cache the file icon
    FileToUpload ftup;
    converter->encodeFile(fileName, ftup);
    if(ftup.offset == 0) {
        QMessageBox::critical(this, "Error", "Unable to open file: " + fileName);
        return;
    } else {
        uploadMap[fileName] = ftup;
        flickrAPI->uploadFile(ftup.getFileName(), ftup.byteArray, fileName);
    }
//    for(QMap<QString, QByteArray>::Iterator part = fileParts.begin(); part != fileParts.end(); ++part) {
//        flickrAPI->uploadFile(part.key(), part.value());
//    }
}

void MainWindow::fileUploaded(const FileDescription &fd, const QString &fileName) {
    if(fd.id.isEmpty()) {
        QMessageBox::critical(this, "Error", "Unable to upload file: " + fileName);
        uploadMap.remove(fileName);
    } else {
        FileToUpload &ftup = uploadMap[fileName];
        BigFileDescription &bfd = uploadFilePartMap[fileName];
        bfd.append(fd);
        if(ftup.offset == -1) {
            uploadMap.remove(fileName);
            flickrFileView->addFile(bfd);
            uploadFilePartMap.remove(fileName);
        } else {
            converter->encodeFile(fileName, ftup);
            flickrAPI->uploadFile(ftup.getFileName(), ftup.byteArray, fileName);
        }
    }
}

//void MainWindow::showFileInfo(FileDescription fd) {
//    flickrFileView->addFile(BigFileDescription(fd));
//}

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

void MainWindow::fileListLoaded(QList<BigFileDescription> files) {
    cDialog->setText("Initializing...");
    for(int i = 0; i < files.size(); ++i) {
        files[i][0].icon = getFileIcon(files[i][0].getCroppedName());
    }
    cDialog->hide();
    flickrFileView->setFileList(files);
}

//----------------------------------------------------------------------------------

void MainWindow::downloadFile(const BigFileDescription &fd) {
    QString downloadFileName = QFileDialog::getSaveFileName(this, "Download file", fd.at(0).getCroppedName());
    if(downloadFileName.isEmpty()) return;

    if(QFile::exists(downloadFileName)) {
        QFile tmp(downloadFileName);
        tmp.remove();
    }

    flickrAPI->getFile(fd.first(), downloadFileName);
    downloadFileMap[downloadFileName] = fd.mid(1);
}

void MainWindow::fileDownloaded(const QByteArray &content, const QString &fileName) {
    BigFileDescription &fd = downloadFileMap[fileName];
    if(!converter->decodeFile(fileName, content)) {
        QMessageBox::critical(this, "Error", "Unable to decode file: " + fileName);
    } else {
        if(fd.isEmpty()) {
            statusBar()->showMessage("File downloaded");
            downloadFileMap.remove(fileName);
        } else {
            flickrAPI->getFile(fd.first(), fileName);
            fd.pop_front();
        }
    }
}

//----------------------------------------------------------------------------------

void MainWindow::uploadTriggered() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select files");
    for(QStringList::Iterator fn = fileNames.begin(); fn != fileNames.end(); ++fn) uploadFile(*fn);
}

//----------------------------------------------------------------------------------

#ifdef Q_WS_WIN

#include <windows.h>
#include <shellapi.h>
#include <QFileIconProvider>

QIcon MainWindow::getFileIcon(const QString &fileName) const {
    int p = fileName.lastIndexOf('.');
    if(p < 0 || p == fileName.size() - 1) {
        QFileIconProvider fip;
        return fip.icon(QFileIconProvider::File);
    }
    QString ext = fileName.right(fileName.size() - p - 1);

    QDir cacheDir("cache");
    if(!cacheDir.exists()) QDir::current().mkdir("cache");
    if(cacheDir.exists(ext)) return QIcon(QPixmap("cache/" + ext, "PNG"));

    if(!QFile::exists(fileName)) {
        QFileIconProvider fip;
        return fip.icon(QFileIconProvider::File);
    }

    SHFILEINFO fi;
    QString pcopy(fileName);
    pcopy.replace('/', '\\');
    wchar_t *fn = new wchar_t[pcopy.size() + 1];
    fn[pcopy.size()] = 0;
    pcopy.toWCharArray(fn);
    SHGetFileInfo(fn, 0, &fi, sizeof(fi), SHGFI_ICON);
    delete[] fn;

    QPixmap img = QPixmap::fromWinHICON(fi.hIcon);
    img.save("cache/" + ext, "PNG");
    return QIcon(img);
}

#else

QIcon MainWindow::getFileIcon(const QString &fileName) const {
    QFileIconProvider fip;
    return fip.icon(QFileIconProvider::File);
}

#endif

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
//    loadingIconTimer->stop();
//    loadingIcon->deleteLater();
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
