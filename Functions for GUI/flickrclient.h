#ifndef FLICKRCLIENT_H
#define FLICKRCLIENT_H


#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QString>


class FlickrClient : public QMainWindow
{
    Q_OBJECT
public:
    explicit FlickrClient(QWidget *parent = 0);
    ~FlickrClient();
    void EncodeFile( QString filename );

public slots:
    void UploadFile_clicked(  );


private:


    QTableWidget* createFilesTable();

    QTableWidget* FilesTab;
    QLabel* FilesTab_label;
    QPushButton* UploadFile;
    QPushButton* DownloadFile;
    QIcon iconCopy;
    QIcon iconCut;
    QIcon iconPaste;
    QIcon iconDel;

private slots:
    void openFileOfItem(int row, int /* column */);





};



#endif // FLICKRCLIENT_H
