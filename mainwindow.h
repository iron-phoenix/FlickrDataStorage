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

//----------------------------------------------------------------------------------

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void uploadFile(const QString &fileName);
    void downloadFile(const BigFileDescription &fd);
    void deleteFile(const BigFileDescription &fd);

    void authResult(bool res);
    void fileUploaded(const FileDescription &id, const QString &fileName);
    void fileDownloaded(const QByteArray &content, const QString &fileName);
    void fileListLoaded(QList<BigFileDescription> files);
    void fileDeleted(bool stat, const QString &fileName);

    void updateDownloadProgress(qint64 bytesLoaded, qint64 bytesTotal, const QString &fileName);
    void updateUploadProgress(qint64 bytesLoaded, qint64 bytesTotal, const QString &fileName);

    void loginUser();
    void logoutUser();
    void uploadTriggered();

    void showLoginLinkMenu(const QString &link);

private:
    QIcon getFileIcon(const QString &fileName) const;

    FlickrAPI *flickrAPI;
    ConnectingDialog *cDialog;
    JPEGConverter *converter;

    FlickrFileView *flickrFileView;
    QAction *actLogin, *actUpload, *actExit, *actKeepLoggedIn;
    QLabel *lbUserID, *lbDownloading, *lbUploading;
    QProgressBar *pbDownloading, *pbUploading;
    QPoint mouseClickPos;

    QMap<QString, FileToUpload> uploadMap;
    QMap<QString, BigFileDescription> uploadFilePartMap;
    QMap<QString, BigFileDescription> downloadFileMap;
    QMap<QString, BigFileDescription> deleteFileMap;
    QMap<QString, quint64> downloadSizeMap, currentDownloadSizeMap;
    QMap<QString, quint64> uploadSizeMap, currentUploadSizeMap;

    quint64 sizeToDownload, sizeDownloaded;
    quint64 sizeToUpload, sizeUploaded;
};

#endif // MAINWINDOW_H
