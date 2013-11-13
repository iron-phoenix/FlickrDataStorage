#include "flickrfileview.h"

#include <QMenu>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QToolTip>

FlickrFileView::FlickrFileView(QWidget *parent) : QListView(parent) {
    actGetFile = new QAction("Download", this);
    connect(actGetFile, SIGNAL(triggered()), this, SLOT(downloadFileTriggered()));

    actDelFile = new QAction("Delete", this);
    connect(actDelFile, SIGNAL(triggered()), this, SLOT(deleteFileTriggered()));

    actUploadFile = new QAction("Upload new file", this);
    connect(actUploadFile, SIGNAL(triggered()), this, SLOT(uploadFileTriggered()));

    this->setViewMode(QListView::IconMode);
    fileModel = new QStandardItemModel(0, 1);

    this->setWordWrap(true);
    this->setGridSize(QSize(120, 70));
    this->setMovement(QListView::Snap);
    this->setResizeMode(QListView::Adjust);
    this->setModel(fileModel);
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
    }
}

void FlickrFileView::addFile(const BigFileDescription &fd) {
    fileList[fd.at(0).id] = fd;
    QStandardItem *item = new QStandardItem(fd.at(0).icon, fd.at(0).getCroppedName());
    item->setData(fd.at(0).id);
    int idx = fileModel->rowCount();
    fileModel->setRowCount(idx + 1);
    fileModel->setItem(idx, item);
}

void FlickrFileView::deleteFile(const QString &id) {
    QMap<QString, BigFileDescription>::Iterator fi = fileList.find(id);
    if(fi != fileList.end()) {
        QList<QStandardItem*> names = fileModel->findItems(fi->at(0).getCroppedName());
        QList<QStandardItem*>::Iterator fn = names.begin();
        for(; fn != names.end(); ++fn) {
            if((*fn)->data().toString() == id) break;
        }
        if(fn != names.end()) {
            int row = (*fn)->index().row();
            fileModel->removeRow(row);
            fileList.erase(fi);
        }
    }
}

void FlickrFileView::deleteAll() {
    fileList.clear();
    fileModel->clear();
}

void FlickrFileView::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    menu.addAction(actGetFile);
    menu.addAction(actDelFile);
    menu.addSeparator();
    menu.addAction(actUploadFile);

    actGetFile->setEnabled(this->indexAt(event->pos()).isValid());
    actDelFile->setEnabled(this->indexAt(event->pos()).isValid());
    menu.exec(event->globalPos());
}

void FlickrFileView::mouseDoubleClickEvent(QMouseEvent *event) {
    QModelIndex idx = this->indexAt(event->pos());
    if(idx.isValid()) {
        QString id = fileModel->data(idx, Qt::UserRole + 1).toString();
        emit requestDownload(fileList[id]);
        event->accept();
    }
}

void FlickrFileView::keyReleaseEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Delete) {
        deleteFileTriggered();
        event->accept();
    }
}

bool FlickrFileView::event(QEvent *e) {
    if(e->type() == QEvent::ToolTip) {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QModelIndex idx = this->indexAt(he->pos());
        if(idx.isValid()) {
            QString id = fileModel->data(idx, Qt::UserRole + 1).toString();
            FileDescription &fd = fileList[id][0];
            QString text = QString("<b>File: </b>%1<br><b>ID: </b>%2<br><b>Uploaded: </b>%3").arg(fd.getCroppedName()).arg(fd.id).arg(fd.uploadDate.toString(Qt::ISODate));
            QToolTip::showText(he->globalPos(), text);
        } else {
            QToolTip::hideText();
        }
        e->accept();
        return true;
    }
    return QListView::event(e);
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
        emit requestDelete(fileList[id]);
    }
}

void FlickrFileView::uploadFileTriggered() {
    emit requestUpload();
}
