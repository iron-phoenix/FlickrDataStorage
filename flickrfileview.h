#ifndef FLICKRFILEVIEW_H
#define FLICKRFILEVIEW_H

#include <QTableView>
#include <QAction>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QStandardItemModel>

#include "flickrapi.h"

class FlickrFileView : public QTableView {
    Q_OBJECT

public:
    FlickrFileView(QWidget *parent);

    void setFileList(const QList<FileDescription> &list);
    void addFile(FileDescription &fd);

signals:
    void requestDownload(FileDescription fd);
    void requestDelete(FileDescription fd);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void resizeEvent(QResizeEvent *event);

private slots:
    void downloadFileTriggered();
    void deleteFileTriggered();

private:
    QAction *actGetFile, *actDelFile;
    QStandardItemModel *fileModel;
    QList<FileDescription> fileList;
};

#endif // FLICKRFILEVIEW_H
