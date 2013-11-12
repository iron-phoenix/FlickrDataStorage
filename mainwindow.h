#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QAction>

#include <QSplashScreen>
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
    void downloadFile(const FileDescription &fd);

    void authResult(bool res);
    void fileUploaded(QString id);
    void fileDownloaded(QByteArray content);
    void fileListLoaded(QList<FileDescription> files);
    void showFileInfo(FileDescription fd);

    void loginUser();
    void logoutUser();
    void uploadTriggered();

private:
    FlickrAPI *flickrAPI;
    ConnectingDialog *cDialog;
    JPEGConverter *converter;

    QTreeView *dirView;
    FlickrFileView *flickrFileView;
    QAction *actLogin, *actUpload;
    QLabel *lbUserID;
    QString downloadFileName;
};

#endif // MAINWINDOW_H
