#include "flickrclient.h"

#include <QGridLayout>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>
#include <QMenu>
#include <QMessageBox>
#include <QHeaderView>


FlickrClient::FlickrClient(QWidget *parent) :
    QMainWindow(parent)
{

    iconCopy = QIcon(":/edit_copy.svg");
    iconCut = QIcon(":/edit_cut.svg");
    iconPaste = QIcon(":/edit_paste.svg");
    iconDel = QIcon(":/edit_delete.svg");

    FilesTab = createFilesTable();
    FilesTab_label = new QLabel( tr( "List of Files" ) );
    DownloadFile = new QPushButton( tr( "Download File" ) );
    UploadFile = new QPushButton( tr( "Upload File") );

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget( FilesTab_label, 0, 0 );
    mainLayout->addWidget( FilesTab, 1, 0 );

    QGridLayout* buttonLayout = new QGridLayout;
    buttonLayout->addWidget( DownloadFile, 0, 0 );
    buttonLayout->addWidget( UploadFile, 0, 1 );
    mainLayout->addLayout( buttonLayout, 2, 0 );

    //setLayout(  );
    QWidget* Central_Widget = new QWidget;
    Central_Widget->setLayout( mainLayout );
    setCentralWidget( Central_Widget );
    connect( UploadFile, SIGNAL( clicked() ), this, SLOT( UploadFile_clicked() ) );

    setWindowTitle( tr(" FlicrClient ") );


    connect(FilesTab->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentRowChanged(QModelIndex,QModelIndex)));

}

QTableWidget* FlickrClient::createFilesTable()
 {
     QTableWidget* filesTable = new QTableWidget(0, 2);
     filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

     QStringList labels;
     labels << tr("File Name") << tr("Size");
     filesTable->setHorizontalHeaderLabels(labels);
     //filesTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
     filesTable->verticalHeader()->hide();
     filesTable->setShowGrid(false);

     connect(filesTable, SIGNAL( cellActivated(int,int) ),
             this, SLOT( openFileOfItem( int, int ) ) );
     return filesTable;
 }



void FlickrClient::openFileOfItem(int row, int /* column */)
 {
     QTableWidgetItem *item = FilesTab->item(row, 0);


    //Do something with select Item

 }



void FlickrClient::UploadFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));

    EncodeFile( filename );

    // todo Send encoded file to Flickr server
}


void FlickrClient::EncodeFile( QString file_name )
{


}

FlickrClient::~FlickrClient()
{

}
