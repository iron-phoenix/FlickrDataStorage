#include "flickrfileview.h"

#include <QMenu>
#include <QHeaderView>

Q_DECLARE_METATYPE(const BigFileDescription*)

FlickrFileView::FlickrFileView(QWidget *parent) : QListView(parent) {
    actGetFile = new QAction("Download", this);
    connect(actGetFile, SIGNAL(triggered()), this, SLOT(downloadFileTriggered()));

    actDelFile = new QAction("Delete", this);
    connect(actDelFile, SIGNAL(triggered()), this, SLOT(deleteFileTriggered()));

    this->setViewMode(QListView::IconMode);
    fileModel = new QStandardItemModel(0, 1);

    this->setWordWrap(true);
    this->setGridSize(QSize(120, 70));
    this->setMovement(QListView::Snap);
    this->setResizeMode(QListView::Adjust);

    this->setModel(fileModel);
//    this->setSelectionBehavior(QTableView::SelectRows);
//    this->setSelectionMode(QTableView::SingleSelection);
//    this->verticalHeader()->hide();
//    this->horizontalHeader()->setStretchLastSection(true);
}

void FlickrFileView::setFileList(const QList<BigFileDescription> &list) {
    fileList.clear();
    fileModel->setRowCount(list.size());
    for(int i = 0; i < list.size(); ++i) {
        const FileDescription &fd = list.at(i).at(0);
        fileList[fd.id] = list.at(i);
        QStandardItem *item = new QStandardItem(fd.icon, fd.getCroppedName());
        item->setData(fd.id);
        fileModel->setItem(i, item);
//        fileModel->setData(fileModel->index(i, 0), QVariant::fromValue((void*)&list.at(i)), Qt::UserRole);
    }
}

void FlickrFileView::addFile(const BigFileDescription &fd) {
    fileList[fd.at(0).id] = fd;
    QStandardItem *item = new QStandardItem(fd.at(0).icon, fd.at(0).getCroppedName());
    item->setData(fd.at(0).id);
    int idx = fileModel->rowCount();
    fileModel->setRowCount(idx + 1);
    fileModel->setItem(idx, item);
//    fileModel->setData(fileModel->index(idx, 0), QVariant::fromValue((void*)&fd));
}

void FlickrFileView::contextMenuEvent(QContextMenuEvent *event) {
    if(this->indexAt(event->pos()).isValid()) {
        QMenu menu(this);
        menu.addAction(actGetFile);
        menu.addAction(actDelFile);
        menu.exec(event->globalPos());
    }
}

void FlickrFileView::downloadFileTriggered() {
    if(this->selectedIndexes().isEmpty()) return;
    QModelIndexList indxs = this->selectedIndexes();
    for(QModelIndexList::Iterator i = indxs.begin(); i != indxs.end(); ++i) {
        QString id = fileModel->data(*i, Qt::UserRole + 1).toString();
        emit requestDownload(fileList[id]);
    }
}

void FlickrFileView::deleteFileTriggered() {
    if(this->selectedIndexes().isEmpty()) return;
    QModelIndexList indxs = this->selectedIndexes();
    for(QModelIndexList::Iterator i = indxs.begin(); i != indxs.end(); ++i) {
        QString id = fileModel->data(*i, Qt::UserRole + 1).toString();
        emit requestDownload(fileList[id]);
    }
}
