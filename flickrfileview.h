#ifndef FLICKRFILEVIEW_H
#define FLICKRFILEVIEW_H

#include <QTableView>
#include <QAction>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QListView>
#include <QMutex>

#include "flickrapi.h"

class FlickrFileView : public QListView {
    Q_OBJECT

public:
    FlickrFileView(QWidget *parent);

    void setFileList(const QList<BigFileDescription> &list);
    void addFile(const BigFileDescription &fd);
    void deleteFile(const QString &id);
    void deleteAll();

signals:
    void requestDownload(const BigFileDescription &fd);
    void requestDelete(const BigFileDescription &fd);
    void requestUpload();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    bool event(QEvent *e);

private slots:
    void downloadFileTriggered();
    void deleteFileTriggered();
    void uploadFileTriggered();

private:
    QAction *actGetFile, *actDelFile, *actUploadFile;
    QStandardItemModel *fileModel;
    QMap<QString, BigFileDescription> fileList;
    QMutex fileListLock;    //add some thread safety, just for example :)
};

#endif // FLICKRFILEVIEW_H
