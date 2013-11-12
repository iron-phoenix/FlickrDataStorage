#ifndef FLICKRFILEVIEW_H
#define FLICKRFILEVIEW_H

#include <QTableView>
#include <QAction>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QListView>

#include "flickrapi.h"

class FlickrFileView : public QListView {
    Q_OBJECT

public:
    FlickrFileView(QWidget *parent);

    void setFileList(const QList<BigFileDescription> &list);
    void addFile(const BigFileDescription &fd);

signals:
    void requestDownload(const BigFileDescription &fd);
    void requestDelete(FileDescription fd);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void downloadFileTriggered();
    void deleteFileTriggered();

private:
    QAction *actGetFile, *actDelFile;
    QStandardItemModel *fileModel;
    QMap<QString, BigFileDescription> fileList;
};

#endif // FLICKRFILEVIEW_H
