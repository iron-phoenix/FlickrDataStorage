#include "flickrfileview.h"

#include <QMenu>
#include <QHeaderView>

FlickrFileView::FlickrFileView(QWidget *parent) : QTableView(parent) {
    actGetFile = new QAction("Download", this);
    connect(actGetFile, SIGNAL(triggered()), this, SLOT(downloadFileTriggered()));

    actDelFile = new QAction("Delete", this);
    connect(actDelFile, SIGNAL(triggered()), this, SLOT(deleteFileTriggered()));
    actDelFile->setEnabled(false);

    QStringList hlabels; hlabels << "Name" << "Upload Date";
    fileModel = new QStandardItemModel(0, 2);
    fileModel->setHorizontalHeaderLabels(hlabels);

    this->setModel(fileModel);
    this->setSelectionBehavior(QTableView::SelectRows);
    this->setSelectionMode(QTableView::SingleSelection);
    this->verticalHeader()->hide();
//    this->horizontalHeader()->setStretchLastSection(true);
}

void FlickrFileView::setFileList(const QList<FileDescription> &list) {
    fileList = list;
    fileModel->setRowCount(list.size());
    for(int i = 0; i < list.size(); ++i) {
        const FileDescription &fd = list.at(i);
        fileModel->setItem(i, 0, new QStandardItem(fd.title.left(fd.title.lastIndexOf('.'))));
        fileModel->setItem(i, 1, new QStandardItem(fd.uploadDate.toString()));
    }
}

void FlickrFileView::addFile(FileDescription &fd) {
    fileList.append(fd);
    QList<QStandardItem*> r;
    r.append(new QStandardItem(fd.title.left(fd.title.lastIndexOf('.'))));
    r.append(new QStandardItem(fd.uploadDate.toString()));
    fileModel->appendRow(r);
}

void FlickrFileView::contextMenuEvent(QContextMenuEvent *event) {
    if(!this->selectedIndexes().isEmpty()) {
        QMenu menu(this);
        menu.addAction(actGetFile);
        menu.addAction(actDelFile);
        menu.exec(event->globalPos());
    }
}

void FlickrFileView::resizeEvent(QResizeEvent *event) {
    this->setColumnWidth(1, 190);
    this->setColumnWidth(0, width() - 195);
}

void FlickrFileView::downloadFileTriggered() {
    if(this->selectedIndexes().isEmpty()) return;
    QModelIndexList indxs = this->selectedIndexes();
    emit requestDownload(fileList.at(indxs[0].row()));
//    for(QModelIndexList::Iterator i = indxs.begin(); i != indxs.end(); ++i) {
//        emit requestDownload(fileList.at(i->row()));
//    }
}

void FlickrFileView::deleteFileTriggered() {
    if(this->selectedIndexes().isEmpty()) return;
    QModelIndexList indxs = this->selectedIndexes();
    for(QModelIndexList::Iterator i = indxs.begin(); i != indxs.end(); ++i) {
        emit requestDelete(fileList.at(i->row()));
    }
}
