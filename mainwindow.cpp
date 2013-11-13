#include "mainwindow.h"

#include <QDebug>
#include <QSettings>
#include <QHBoxLayout>
#include <QFile>
#include <QMenuBar>
#include <QMenu>
#include <QHeaderView>
#include <QFileDialog>
#include <QStatusBar>
#include <QMessageBox>
#include <QIcon>
#include <QPixmap>
#include <QFileIconProvider>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    flickrAPI = new FlickrAPI(this);
    connect(flickrAPI, SIGNAL(authResult(bool)), this, SLOT(authResult(bool)));
    connect(flickrAPI, SIGNAL(fileListLoaded(QList<BigFileDescription>)), this, SLOT(fileListLoaded(QList<BigFileDescription>)));
    connect(flickrAPI, SIGNAL(fileUploaded(FileDescription, QString)), this, SLOT(fileUploaded(FileDescription, QString)));
    connect(flickrAPI, SIGNAL(fileDownloaded(QByteArray,QString)), this, SLOT(fileDownloaded(QByteArray,QString)));
    connect(flickrAPI, SIGNAL(fileDeleted(bool,QString)), this, SLOT(fileDeleted(bool,QString)));
    connect(flickrAPI, SIGNAL(downloadProgress(qint64,qint64,QString)), this, SLOT(updateDownloadProgress(qint64,qint64,QString)));
    connect(flickrAPI, SIGNAL(uploadProgress(qint64,qint64,QString)), this, SLOT(updateUploadProgress(qint64,qint64,QString)));

    actUpload = new QAction("Upload files...", this);
    actUpload->setEnabled(false);
    connect(actUpload, SIGNAL(triggered()), this, SLOT(uploadTriggered()));

    actLogin = new QAction("Login", this);
    connect(actLogin, SIGNAL(triggered()), this, SLOT(loginUser()));

    actExit = new QAction("Exit", this);
    connect(actExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    QMenu *fileMenu = menuBar()->addMenu("File");;
    fileMenu->addAction(actLogin);
    fileMenu->addAction(actUpload);
    fileMenu->addSeparator();
    fileMenu->addAction(actExit);

    cDialog = new ConnectingDialog(this);
    converter = new JPEGConverter(":/tmp.jpg");

    pbDownloading = new QProgressBar(this);
    pbDownloading->setRange(0, 100);
    pbDownloading->setMaximumHeight(20);

    pbUploading = new QProgressBar(this);
    pbUploading->setRange(0, 100);
    pbUploading->setMaximumHeight(20);

    lbDownloading = new QLabel(this);
    lbUploading = new QLabel(this);
    statusBar()->addWidget(lbDownloading);
    statusBar()->addWidget(pbDownloading);
    statusBar()->addWidget(lbUploading);
    statusBar()->addWidget(pbUploading);
    lbDownloading->hide();
    pbDownloading->hide();
    lbUploading->hide();
    pbUploading->hide();

    flickrFileView = new FlickrFileView(this);
    connect(flickrFileView, SIGNAL(requestDownload(BigFileDescription)), this, SLOT(downloadFile(BigFileDescription)));
    connect(flickrFileView, SIGNAL(requestDelete(BigFileDescription)), this, SLOT(deleteFile(BigFileDescription)));
    connect(flickrFileView, SIGNAL(requestUpload()), this, SLOT(uploadTriggered()));
    flickrFileView->setEnabled(false);

    this->setCentralWidget(flickrFileView);

    lbUserID = new QLabel("<b>not logged in  </b>", this);
    this->statusBar()->addPermanentWidget(lbUserID);
    this->statusBar()->setSizeGripEnabled(false);

    sizeToDownload = sizeDownloaded = 0;
    sizeToUpload = sizeUploaded = 0;

    this->setWindowTitle("Flickr Data Storage");
    this->resize(500, 400);

//    QTimer::singleShot(500, this, SLOT(loginUser()));
}

MainWindow::~MainWindow() {
}

//----------------------------------------------------------------------------------

#include <QDragEnterEvent>
#include <QDropEvent>

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> fileUrl = event->mimeData()->urls();
    for(QList<QUrl>::Iterator url = fileUrl.begin(); url != fileUrl.end(); ++url) {
        QString urlString = url->toString();
        if(urlString.startsWith("file")) {
            QString name = urlString.mid(8);
            QFileInfo inf(name);
            if(inf.isFile()) uploadFile(name);
        }
    }
}

//----------------------------------------------------------------------------------

void MainWindow::loginUser() {
    cDialog->setText("Logging in...");
    cDialog->show();

    QSettings appSettings("settings.ini", QSettings::IniFormat, this);
    QString token = appSettings.value("auth_token").toString();
    QString secret = appSettings.value("auth_secret").toString();
    flickrAPI->loginUser(token, secret);
}

void MainWindow::logoutUser() {
    QSettings appSettings("settings.ini", QSettings::IniFormat, this);
    appSettings.setValue("auth_token", "");
    appSettings.setValue("auth_secret", "");
    lbUserID->setText("<b>not logged in</b>  ");
    flickrFileView->setEnabled(false);
    actUpload->setEnabled(false);
    this->setAcceptDrops(false);

    actLogin->setText("Login");
    actLogin->disconnect(this, SLOT(logoutUser()));
    connect(actLogin, SIGNAL(triggered()), this, SLOT(loginUser()));

    flickrFileView->deleteAll();
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

    if(!uploadMap.empty()) {
        pbUploading->show();
        lbUploading->show();
    }
}

void MainWindow::fileUploaded(const FileDescription &fd, const QString &fileName) {
    if(fd.id.isEmpty()) {
        QMessageBox::critical(this, "Error", "Unable to upload file: " + fileName);
        uploadMap.remove(fileName);
    } else {
        FileToUpload &ftup = uploadMap[fileName];
        BigFileDescription &bfd = uploadFilePartMap[fileName];
        bfd.append(fd);

        sizeToUpload -= uploadSizeMap[fileName];
        sizeUploaded -= currentUploadSizeMap[fileName];
        uploadSizeMap.remove(fileName);
        currentUploadSizeMap.remove(fileName);

        if(ftup.offset == -1) {
            uploadMap.remove(fileName);
            bfd[0].icon = getFileIcon(bfd[0].getCroppedName());
            flickrFileView->addFile(bfd);
            uploadFilePartMap.remove(fileName);
        } else {
            converter->encodeFile(fileName, ftup);
            flickrAPI->uploadFile(ftup.getFileName(), ftup.byteArray, fileName);
        }
    }

    if(uploadMap.empty()) {
        pbUploading->hide();
        lbUploading->hide();
    }
}

//----------------------------------------------------------------------------------

void MainWindow::authResult(bool res) {
    if(res) {
        actLogin->setText("Logout");
        actLogin->disconnect(this, SLOT(loginUser()));
        connect(actLogin, SIGNAL(triggered()), this, SLOT(logoutUser()));

        this->setAcceptDrops(true);
        flickrFileView->setEnabled(true);
        actUpload->setEnabled(true);
        lbUserID->setText(QString("Logged in as <b>%1</b>  ").arg(flickrAPI->getUsername()));

        QSettings appSettings("settings.ini", QSettings::IniFormat, this);
        appSettings.setValue("auth_token", flickrAPI->getAuthToken());
        appSettings.setValue("auth_secret", flickrAPI->getAuthSecret());
        cDialog->setText("Loading file list...");
        flickrAPI->getFileList();
    } else {
        cDialog->hide();
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

    if(downloadSizeMap.isEmpty()) {
        pbDownloading->setValue(0);
        lbDownloading->setText("Downloading files: 1/0  ");
        pbDownloading->show();
        lbDownloading->show();
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
            sizeToDownload -= downloadSizeMap[fileName];
            sizeDownloaded -= currentDownloadSizeMap[fileName];
            downloadSizeMap.remove(fileName);
            currentDownloadSizeMap.remove(fileName);
            if(downloadSizeMap.empty()) {
                pbDownloading->hide();
                lbDownloading->hide();
            }

            statusBar()->showMessage("File downloaded: " + fileName, 3000);
            downloadFileMap.remove(fileName);
        } else {
            flickrAPI->getFile(fd.first(), fileName);
            fd.removeFirst();
        }
    }
}

//----------------------------------------------------------------------------------

void MainWindow::uploadTriggered() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select files");
    for(QStringList::Iterator fn = fileNames.begin(); fn != fileNames.end(); ++fn) uploadFile(*fn);
}

//----------------------------------------------------------------------------------

void MainWindow::updateDownloadProgress(qint64 bytesLoaded, qint64 bytesTotal, const QString &fileName) {
    QMap<QString, quint64>::Iterator fn = downloadSizeMap.find(fileName);
    if(fn == downloadSizeMap.end()) {
        downloadSizeMap[fileName] = bytesTotal;
        sizeToDownload += bytesTotal;
    }
    sizeDownloaded += bytesLoaded - currentDownloadSizeMap[fileName];
    currentDownloadSizeMap[fileName] = bytesLoaded;

    int total = 0;
    for(QMap<QString, BigFileDescription>::ConstIterator df = downloadFileMap.begin(); df != downloadFileMap.end(); ++df) {
        total += df.value().size();
    }
    QString dfcount = QString("Downloading: %1/%2   ").arg(downloadFileMap.size()).arg(total);

    lbDownloading->setText(dfcount);
    pbDownloading->setValue( ((double)sizeDownloaded / (double)sizeToDownload) * 100 );
}

void MainWindow::updateUploadProgress(qint64 bytesLoaded, qint64 bytesTotal, const QString &fileName) {
    QMap<QString, quint64>::Iterator fn = uploadSizeMap.find(fileName);
    if(fn == uploadSizeMap.end()) {
        uploadSizeMap[fileName] = bytesTotal;
        sizeToUpload += bytesTotal;
    }
    sizeUploaded += bytesLoaded - currentUploadSizeMap[fileName];
    currentUploadSizeMap[fileName] = bytesLoaded;

    int total = 0;
    for(QMap<QString, BigFileDescription>::ConstIterator df = uploadFilePartMap.begin(); df != uploadFilePartMap.end(); ++df) {
        total += df.value().size();
    }
    QString dfcount = QString("Uploading: %1/%2   ").arg(uploadFilePartMap.size()).arg(total);

    lbUploading->setText(dfcount);
    pbUploading->setValue( ((double)sizeUploaded / (double)sizeToUpload) * 100 );
}

//----------------------------------------------------------------------------------

void MainWindow::deleteFile(const BigFileDescription &fd) {
    if(QMessageBox::Yes == QMessageBox::question(this, "Delete file", QString("Do you really want to delete %1?").arg(fd[0].getCroppedName()), QMessageBox::Yes, QMessageBox::No)) {
        flickrAPI->deleteFile(fd.first(), fd[0].id);
        deleteFileMap[fd[0].id] = fd.mid(1);
        qDebug() << "delete file triggered";
    }
}

void MainWindow::fileDeleted(bool stat, const QString &fileName) {
    BigFileDescription &fd = downloadFileMap[fileName];
    if(stat) {
        if(fd.empty()) {
            flickrFileView->deleteFile(fileName);
            deleteFileMap.remove(fileName);
            statusBar()->showMessage("File deleted", 2000);
        } else {
            flickrAPI->deleteFile(fd.first(), fileName);
            fd.removeFirst();
        }
    } else {
        QString name = fd.isEmpty() ? QString("ID " + fileName) : fd.first().getCroppedName();
        QMessageBox::critical(this, "Error", name);
        deleteFileMap.remove(fileName);
    }
}

//----------------------------------------------------------------------------------

#ifdef Q_WS_WIN

#include <windows.h>
#include <shellapi.h>

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
