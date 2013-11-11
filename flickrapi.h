#ifndef FLICKRAPI_H
#define FLICKRAPI_H

#include <QObject>
#include <QWebView>
#include <QNetworkAccessManager>
#include <QMap>
#include <QList>
#include <QUrl>

struct FileDescription {
    QString id;
    QString secret;
    QString server;
    QString title;
    QString farm;
    QString format;
};

class FlickrAPI : public QObject {
    Q_OBJECT

public:
    FlickrAPI(QObject *parent);

    void loginUser(const QString &token = "", const QString &tokenSecret = "");
    void uploadFile(const QString &name, const QByteArray &data, const QString &fileType = "bmp");
    void getFileList(int page = -1);
    void getFile(const FileDescription &fd);

signals:
    void authResult(bool);
    void fileUploaded(QString);
    void fileListLoaded(QList<FileDescription>);

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

    QByteArray getReplyContent(QObject *rawReply) const;
    QMap<QString, QString> parseReply(const QString &reply) const;
    QString hmacSha1(QByteArray key, const QByteArray &baseString) const;
    quint64 timestamp() const;
    QString nonce() const;

    QNetworkAccessManager *netManager;
    QWebView *webView;
    QString oauthToken, oauthTokenSecret, oauthVerifier;
    QString consumerKey, consumerSecret;
    QString userID, userName;
    QList<FileDescription> fileList;
};

#endif // FLICKRAPI_H
