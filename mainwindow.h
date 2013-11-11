#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "flickrapi.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void uploadFile(const QString &fileName);

    void authResult(bool res);
    void fileUploaded(QString id);
    void fileListLoaded(QList<FileDescription> files);

private:
    FlickrAPI *flickrAPI;
};

#endif // MAINWINDOW_H
