#ifndef FLICKRAPI_H
#define FLICKRAPI_H

#include <QObject>
#include <QWebView>
#include <QNetworkAccessManager>
#include <QMap>
#include <QList>
#include <QDateTime>
#include <QUrl>
#include <QIcon>

struct FileDescription {
    FileDescription() {}

    QString getCroppedName() const {
        return title.left(title.indexOf(QRegExp("\\.part\\d+\\.jpeg\\b")));
//        return title.left(title.lastIndexOf('.'));
    }

    bool operator<(const FileDescription &other) const {
        return title < other.title;
    }

    QString id;
    QString secret;
    QString server;
    QString title;
    QString farm;
    QString format;
    QDateTime uploadDate;
    QIcon icon;
};

typedef QList<FileDescription> BigFileDescription;

class FlickrAPI : public QObject {
    Q_OBJECT

public:
    FlickrAPI(QObject *parent);

    void loginUser(const QString &token = "", const QString &tokenSecret = "");
    void uploadFile(const QString &name, const QByteArray &data, const QString &sourceFileName);
    void getFileList(int page = -1);
    void getFile(const FileDescription &fd, const QString &id);
    void getFileInfo(const QString &id, const QString &fileName);

    bool isLoggedIn() const {
        return !userID.isEmpty() && !userName.isEmpty();
    }

    QString getAuthToken() const {
        return oauthToken;
    }

    QString getAuthSecret() const {
        return oauthTokenSecret;
    }

    QString getUserID() const {
        return userID;
    }

    QString getUsername() const {
        return userName;
    }

signals:
    void authResult(bool);
    void fileUploaded(FileDescription, QString);
    void fileListLoaded(QList<BigFileDescription>);
    void fileInfoLoaded(FileDescription);
    void fileDownloaded(QByteArray, QString);

private slots:
    void replyUploadError();
    void replyDownloadError();
    void replyAuthError();
    void cancelLogin();
    void replyRequestTokenFinished();
    void replyAccessTokenFinished();
    void replyTestLogin();
    void replyUploadFinished();
    void replyGetFileListFinished();
    void replyGetFileFinished();
    void replyGetFileInfoFinished();
    void redirected(QUrl);

private:
    void getRequestToken();
    void getAccessToken();
    void showAuthDialog();
    void testLogin();

    QString baseStringFromURL(const QString &method, const QMap<QString, QString> &params, const QString &httpMethod = "GET") const;
    QString urlFromParams(const QString &url, const QMap<QString, QString> &params) const;
    void setDefaultOAuthParams(const QString &url, QMap<QString, QString> &params, const QString &httpMethod = "GET") const;
    QString oauthHeader(const QString &method,const QMap<QString, QString> &params) const;

    inline QString extractReplyID(QObject *rawReply, QMap<QNetworkReply*, QString> &fromMap);
    QByteArray getReplyContent(QObject *rawReply) const;
    QMap<QString, QString> parseReply(const QString &reply) const;
    QString hmacSha1(QByteArray key, const QByteArray &baseString) const;
    quint64 timestamp() const;
    QString nonce() const;

    QList<BigFileDescription> processFiles(QList<FileDescription> &flist);

    QNetworkAccessManager *netManager;
    QWebView *webView;
    QString oauthToken, oauthTokenSecret, oauthVerifier;
    QString consumerKey, consumerSecret;
    QString userID, userName;
    QList<FileDescription> fileList;
    QMap<QNetworkReply*, QString> fileUploadReplyMap;
    QMap<QNetworkReply*, QString> fileDownloadReplyMap;
};

#endif // FLICKRAPI_H
