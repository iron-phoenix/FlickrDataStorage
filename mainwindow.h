#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QAction>

#include <QSplashScreen>
#include <QProgressBar>
#include <QMovie>
#include <QTimer>
#include <QLabel>

#include "flickrapi.h"
#include "flickrfileview.h"
#include "JPEGConverter.h"

class ConnectingDialog : public QSplashScreen {
    Q_OBJECT

public:
    ConnectingDialog(QWidget *parent = 0);
    ~ConnectingDialog();

public slots:
    void setText(const QString &txt);

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

private:
    QMovie *loadingIcon;
    QTimer *loadingIconTimer;
    QLabel *lbText;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void uploadFile(const QString &fileName);
    void downloadFile(const BigFileDescription &fd);
    void deleteFile(const BigFileDescription &fd);

    void authResult(bool res);
    void fileUploaded(const FileDescription &id, const QString &fileName);
    void fileDownloaded(const QByteArray &content, const QString &fileName);
    void fileListLoaded(QList<BigFileDescription> files);

    void updateDownloadProgress(qint64 bytesLoaded, qint64 bytesTotal, const QString &fileName);

    void loginUser();
    void logoutUser();
    void uploadTriggered();

private:
    QIcon getFileIcon(const QString &fileName) const;
    QString getDownloadingFilesCountString() const;

    FlickrAPI *flickrAPI;
    ConnectingDialog *cDialog;
    JPEGConverter *converter;

    QTreeView *dirView;
    FlickrFileView *flickrFileView;
    QAction *actLogin, *actUpload;
    QLabel *lbUserID, *lbDownloading;
    QProgressBar *pbDownloading;

    QMap<QString, FileToUpload> uploadMap;
    QMap<QString, BigFileDescription> uploadFilePartMap;
    QMap<QString, BigFileDescription> downloadFileMap;
    QMap<QString, quint64> downloadSizeMap, currentSizeMap;

    quint64 sizeToDownload, sizeDownloaded;
};

#endif // MAINWINDOW_H
