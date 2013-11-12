#include "flickrapi.h"

#include <QCryptographicHash>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtXml>

#include <QDebug>

typedef QMap<QString, QString> ParamMap;

FlickrAPI::FlickrAPI(QObject *parent) : QObject(parent) {
    netManager = new QNetworkAccessManager(this);

    consumerKey = "338139b7fea1b05c90fc084e797eb29f";
    consumerSecret = "4be42e89254842a3";
    oauthToken = "";
    oauthTokenSecret = "";
}

//----------------------------------------------------------------------------------

void FlickrAPI::loginUser(const QString &token, const QString &tokenSecret) {
    oauthToken = token;
    oauthTokenSecret = tokenSecret;

    if(oauthToken.isEmpty() || oauthTokenSecret.isEmpty()) getRequestToken();
    else testLogin();
}

//----------------------------------------------------------------------------------

QString FlickrAPI::oauthHeader(const QString &method, const QMap<QString, QString> &params) const {
    QString r = "OAuth realm=\"" + method + "\"";
    for(QMap<QString, QString>::ConstIterator p = params.begin(); p != params.end(); ++p) {
        if(p.key().startsWith("oauth")) r += "," + p.key() + "=\"" + p.value() + "\"";
    }
    return r;
}

void FlickrAPI::setDefaultOAuthParams(const QString &url, QMap<QString, QString> &params, const QString &httpMethod) const {
    params["oauth_nonce"] = nonce();
    params["oauth_timestamp"] = QString::number(timestamp());
    params["oauth_consumer_key"] = consumerKey;
    params["oauth_signature_method"] = "HMAC-SHA1";
    params["oauth_version"] = "1.0";

    QString hashKey = consumerSecret + "&" + oauthTokenSecret;
    QString baseString = baseStringFromURL(url, params, httpMethod);
    QString sign = QUrl::toPercentEncoding(hmacSha1(hashKey.toAscii(), baseString.toAscii())).constData();
    params["oauth_signature"] = sign;
}

//----------------------------------------------------------------------------------

void FlickrAPI::getRequestToken() {
    QString methodURL = "http://www.flickr.com/services/oauth/request_token";

    QMap<QString, QString> requestParams;
    requestParams["oauth_callback"] = "http%3A%2F%2Fwww.example.com";
    setDefaultOAuthParams(methodURL, requestParams);

    QNetworkRequest req = QNetworkRequest(QUrl(methodURL));
    req.setRawHeader("User-Agent", "Mozilla/5.0");
    req.setRawHeader("Authorization", oauthHeader(methodURL, requestParams).toAscii());

    QNetworkReply *reply = netManager->get(req);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyAuthError()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyRequestTokenFinished()));
}

void FlickrAPI::replyRequestTokenFinished() {
    QByteArray reply = getReplyContent(sender());

    oauthToken.clear();
    oauthTokenSecret.clear();
    ParamMap tokens = parseReply(reply);
    oauthToken = tokens["oauth_token"];
    oauthTokenSecret = tokens["oauth_token_secret"];

    if(oauthToken.isEmpty() || oauthTokenSecret.isEmpty()) emit authResult(false);
    else showAuthDialog();
}

//----------------------------------------------------------------------------------

void FlickrAPI::showAuthDialog() {
    webView = new QWebView();
    webView->setAttribute(Qt::WA_DeleteOnClose);
//    webView->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    webView->setWindowTitle("Logging in...");
    connect(webView, SIGNAL(urlChanged(QUrl)), this, SLOT(redirected(QUrl)));
    connect(webView, SIGNAL(destroyed()), this, SLOT(cancelLogin()));

    webView->show();
    webView->load(QUrl("http://www.flickr.com/services/oauth/authorize?perms=delete&oauth_token=" + oauthToken));
}

void FlickrAPI::redirected(QUrl url) {
    QString urlString = url.toString();
    if(!urlString.contains("www.example.com")) return;

    ParamMap tokens = parseReply(urlString.mid(urlString.indexOf('?') + 1));
    oauthVerifier = tokens["oauth_verifier"];
    //maybe we should check oauth_token

    if(oauthVerifier.isEmpty()) {
        qDebug() << "Unable to get oauth_verifier";
        emit authResult(false);
    } else {
        webView->disconnect(this, SLOT(cancelLogin()));
        webView->close();
        getAccessToken();
    }
}

void FlickrAPI::cancelLogin() {
    oauthToken = "";
    oauthTokenSecret = "";
    emit authResult(false);
}

//----------------------------------------------------------------------------------

void FlickrAPI::getAccessToken() {
    QString methodURL = "http://www.flickr.com/services/oauth/access_token";

    QMap<QString, QString> requestParams;
    requestParams["oauth_verifier"] = oauthVerifier;
    requestParams["oauth_token"] = oauthToken;
    setDefaultOAuthParams(methodURL, requestParams);

    QNetworkRequest req = QNetworkRequest(QUrl(methodURL));
    req.setRawHeader("User-Agent", "Mozilla/5.0");
    req.setRawHeader("Authorization", oauthHeader(methodURL, requestParams).toAscii());

    QNetworkReply *reply = netManager->get(req);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyAuthError()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyAccessTokenFinished()));
}

void FlickrAPI::replyAccessTokenFinished() {
    QByteArray reply = getReplyContent(sender());

    oauthToken.clear();
    oauthTokenSecret.clear();
    ParamMap tokens = parseReply(reply);
    oauthToken = tokens["oauth_token"];
    oauthTokenSecret = tokens["oauth_token_secret"];

    if(oauthToken.isEmpty() || oauthTokenSecret.isEmpty()) {
        qDebug() << "Unable to get Access Token";
        emit authResult(false);
    } else {
        userName = QUrl::fromPercentEncoding(tokens["username"].toAscii());
        userID = QUrl::fromPercentEncoding(tokens["user_nsid"].toAscii());
        qDebug() << "Looged in as" << userName << "id: " << userID;
        qDebug() << oauthToken << oauthTokenSecret;
        emit authResult(true);
    }
}

//----------------------------------------------------------------------------------

void FlickrAPI::testLogin() {
    QString methodURL = "http://api.flickr.com/services/rest";

    QMap<QString, QString> requestParams;
    requestParams["oauth_token"] = oauthToken;
    requestParams["method"] = "flickr.test.login";
    setDefaultOAuthParams(methodURL, requestParams);

    QNetworkRequest req(QUrl(urlFromParams(methodURL, requestParams)));
    req.setRawHeader("User-Agent", "Mozilla/5.0");
    req.setRawHeader("Authorization", oauthHeader("http://api.flickr.com/services/rest", requestParams).toAscii());
//    req.setRawHeader("Authorization", oauthHeader("http://www.flickr.com/services/oauth/access_token", requestParams).toAscii());

    QNetworkReply *reply = netManager->get(req);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyAuthError()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyTestLogin()));
}

void FlickrAPI::replyTestLogin() {
    QByteArray reply = getReplyContent(sender());

    QDomDocument xmlReply;
    if(!xmlReply.setContent(reply)) {
        emit authResult(false);
        return;
    }

    QDomElement curNode = xmlReply.firstChild().nextSibling().toElement();
    if(curNode.attribute("stat") != "ok") {
        emit authResult(false);
        return;
    }
    curNode = curNode.firstChild().toElement();
    userID = curNode.attribute("id");
    userName = curNode.firstChild().toElement().text();

    if(userID.isEmpty() || userName.isEmpty()) {
        emit authResult(false);
        return;
    }

    emit authResult(true);
}

//----------------------------------------------------------------------------------

void FlickrAPI::uploadFile(const QString &name, const QByteArray &data, const QString &sourceFileName) {
    QString methodURL = "http://up.flickr.com/services/upload";

    QMap<QString, QString> requestParams;
//    requestParams["oauth_token"] = oauthToken;
//    requestParams["title"] = name;
//    setDefaultOAuthParams(methodURL, requestParams, "POST");

    requestParams["oauth_nonce"] = nonce();
    requestParams["oauth_timestamp"] = QString::number(timestamp());
    requestParams["oauth_consumer_key"] = consumerKey;
    requestParams["oauth_token"] = oauthToken;
    requestParams["oauth_signature_method"] = "HMAC-SHA1";
    requestParams["oauth_version"] = "1.0";
    requestParams["title"] = name;

    QString hashKey = consumerSecret + "&" + oauthTokenSecret;
    QString baseString = baseStringFromURL("http://up.flickr.com/services/upload", requestParams, "POST");
    QString sign = QUrl::toPercentEncoding(hmacSha1(hashKey.toAscii(), baseString.toAscii())).constData();
    requestParams["oauth_signature"] = sign;

    QString boundary = "---------------------------" + QString::number(qrand()) + QString::number(qrand()) + QString::number(qrand());
    QString endBoundary = "\r\n--" + boundary + "--\r\n";
    QString contentType = "multipart/form-data; boundary=" + boundary;
    boundary = "--" + boundary + "\r\n";

    QByteArray reqData(boundary.toAscii());
    reqData.append(QString("Content-Disposition: form-data; name=\"title\"\r\n\r\n").toAscii());
    reqData.append(name.toAscii());
    boundary = "\r\n" + boundary;
    reqData.append(boundary);
    reqData.append(QString("Content-Disposition: form-data; name=\"photo\"; filename=\"%1\"\r\n").arg(name));
    reqData.append(QString("Content-Type: image/jpeg\r\n\r\n"));
    reqData.append(data);
    reqData.append(endBoundary);

    QNetworkRequest req(QUrl("http://up.flickr.com/services/upload"));
    req.setRawHeader("Host", "up.flickr.com");
    req.setRawHeader("User-Agent", "Mozilla/5.0");
    req.setRawHeader("Authorization", oauthHeader("http://up.flickr.com/services/upload", requestParams).toAscii());
    req.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    req.setHeader(QNetworkRequest::ContentLengthHeader, reqData.size());

    QNetworkReply *reply = netManager->post(req, reqData);
    fileUploadReplyMap[reply] = sourceFileName;
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyUploadError()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyUploadFinished()));
}

void FlickrAPI::replyUploadFinished() {
    QString fileName = extractReplyID(sender(), fileUploadReplyMap);
    QByteArray reply = getReplyContent(sender());
    QDomDocument xmlReply;
    if(!xmlReply.setContent(reply)) {
        emit fileUploaded(FileDescription(), fileName);
        return;
    }

    QDomElement curNode = xmlReply.firstChild().nextSibling().toElement();
    if(curNode.attribute("stat") != "ok") {
        qDebug() << "error" << curNode.firstChild().toElement().attribute("msg");
        emit fileUploaded(FileDescription(), fileName);
        return;
    }

    QString photoID = curNode.firstChild().toElement().text();
    getFileInfo(photoID, fileName);
//    emit fileUploaded(photoID, fileName);
}

//----------------------------------------------------------------------------------

void FlickrAPI::deleteFile(const FileDescription &fd){
    QString methodURL = "http://api.flickr.com/services/rest";
    QMap<QString, QString> requestParams;
    requestParams["oauth_token"] = oauthToken;
    requestParams["method"] = "flickr.photos.delete";
    requestParams["photo_id"] = fd.id;
    setDefaultOAuthParams(methodURL, requestParams);

    QNetworkRequest req(QUrl(urlFromParams(methodURL, requestParams)));
    req.setRawHeader("User-Agent", "Mozilla/5.0");
    req.setRawHeader("Authorization", oauthHeader(methodURL, requestParams).toAscii());

    QNetworkReply *reply = netManager->get(req);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyDownloadError()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyDeleteFileFinished()));
}

void FlickrAPI::replyDeleteFileFinished(){
    QByteArray reply = getReplyContent(sender());

    QDomDocument xmlReply;
    if(!xmlReply.setContent(reply)) {
        emit fileDeleted(false);
        return;
    }
    QDomElement curNode = xmlReply.firstChild().nextSibling().toElement();
    if(curNode.attribute("stat") != "ok") {
        emit fileDeleted(false);
        return;
    }
    emit fileDeleted(true);
}

//----------------------------------------------------------------------------------

void FlickrAPI::getFileList(int page) {
    if(page < 0) fileList.clear();
    QString methodURL = "http://api.flickr.com/services/rest";

    QMap<QString, QString> requestParams;
    requestParams["oauth_token"] = oauthToken;
    requestParams["method"] = "flickr.photos.search";
    requestParams["user_id"] = userID;
    requestParams["extras"] = "original_format,date_upload";
    if(page > 0) requestParams["page"] = QString::number(page);
    setDefaultOAuthParams(methodURL, requestParams);

//    QNetworkRequest req(QUrl(methodURL + "?method=flickr.photos.search&user_id=" + userID));
    QNetworkRequest req(QUrl(urlFromParams(methodURL, requestParams)));
    req.setRawHeader("User-Agent", "Mozilla/5.0");
    req.setRawHeader("Authorization", oauthHeader("http://api.flickr.com/services/rest", requestParams).toAscii());

    QNetworkReply *reply = netManager->get(req);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyDownloadError()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyGetFileListFinished()));
}

void FlickrAPI::replyGetFileListFinished() {
    QByteArray reply = getReplyContent(sender());

    QDomDocument xmlReply;
    if(!xmlReply.setContent(reply)) {
        return;
    }

    QDomElement curNode = xmlReply.firstChild().nextSibling().toElement();
    if(curNode.attribute("stat") != "ok") {
        return;
    }

    curNode = curNode.firstChild().toElement();
    int page = curNode.attribute("page").toInt();
    int pages = curNode.attribute("pages").toInt();

    for(QDomElement photoNode = curNode.firstChild().toElement(); !photoNode.isNull(); photoNode = photoNode.nextSibling().toElement()) {
        FileDescription fd;
        fd.id = photoNode.attribute("id");
        fd.secret = photoNode.attribute("originalsecret");
        fd.server = photoNode.attribute("server");
        fd.title = photoNode.attribute("title");
        fd.farm = photoNode.attribute("farm");
        fd.format = photoNode.attribute("originalformat");
        fd.uploadDate = QDateTime::fromMSecsSinceEpoch(QString(photoNode.attribute("dateupload") + "000").toULongLong());
        fileList.push_back(fd);
    }

    if(page < pages) getFileList(page + 1);
    else emit fileListLoaded(processFiles(fileList));
}

//----------------------------------------------------------------------------------

void FlickrAPI::getFileInfo(const QString &id, const QString &fileName) {
    QString methodURL = "http://api.flickr.com/services/rest";
    QMap<QString, QString> requestParams;
    requestParams["oauth_token"] = oauthToken;
    requestParams["method"] = "flickr.photos.getInfo";
    requestParams["user_id"] = userID;
    requestParams["photo_id"] = id;
    setDefaultOAuthParams(methodURL, requestParams);

    QNetworkRequest req(QUrl(urlFromParams(methodURL, requestParams)));
    req.setRawHeader("User-Agent", "Mozilla/5.0");
    req.setRawHeader("Authorization", oauthHeader(methodURL, requestParams).toAscii());

    QNetworkReply *reply = netManager->get(req);
    fileUploadReplyMap[reply] = fileName;
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyDownloadError()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyGetFileInfoFinished()));
}

void FlickrAPI::replyGetFileInfoFinished() {
    QString fileName = extractReplyID(sender(), fileUploadReplyMap);
    QByteArray reply = getReplyContent(sender());
    QDomDocument xmlReply;
    if(!xmlReply.setContent(reply)) {
        emit fileUploaded(FileDescription(), fileName);
        return;
    }

    QDomElement curNode = xmlReply.firstChild().nextSibling().toElement();
    if(curNode.attribute("stat") != "ok") {
        qDebug() << "error" << curNode.firstChild().toElement().attribute("msg");
        emit fileUploaded(FileDescription(), fileName);
        return;
    }
    curNode = curNode.firstChild().toElement();
    FileDescription fd;
    fd.id = curNode.attribute("id");
    fd.secret = curNode.attribute("originalsecret");
    fd.server = curNode.attribute("server");
//    fd.title = curNode.attribute("title");
    fd.farm = curNode.attribute("farm");
    fd.format = curNode.attribute("originalformat");
    fd.uploadDate = QDateTime::fromMSecsSinceEpoch(QString(curNode.attribute("dateuploaded") + "000").toULongLong());
    for(curNode = curNode.firstChild().toElement(); !curNode.isNull(); curNode = curNode.nextSibling().toElement()) {
        if(curNode.tagName() == "title") {
            fd.title = curNode.text();
            break;
        }
    }
    if(fd.title.isEmpty()) emit fileUploaded(FileDescription(), fileName);
    else emit fileUploaded(fd, fileName);
}

//----------------------------------------------------------------------------------

void FlickrAPI::getFile(const FileDescription &fd, const QString &id) {
    QString fileUrl = QString("http://farm%1.staticflickr.com/%2/%3_%4_o.%5").arg(fd.farm)
            .arg(fd.server).arg(fd.id).arg(fd.secret).arg(fd.format);

    qDebug() << fileUrl;

    QNetworkReply *reply = netManager->get(QNetworkRequest(QUrl(fileUrl)));
    fileDownloadReplyMap[reply] = id;
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyDownloadError()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyGetFileFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(emitDownloadPorgress(qint64,qint64)));
}

void FlickrAPI::replyGetFileFinished() {
    QString fileName = extractReplyID(sender(), fileDownloadReplyMap);
    QByteArray reply = getReplyContent(sender());
    emit fileDownloaded(reply, fileName);
}

//----------------------------------------------------------------------------------

void FlickrAPI::replyUploadError() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->disconnect(this);
    qDebug() << reply->errorString();
    reply->deleteLater();
}

void FlickrAPI::replyDownloadError() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->disconnect(this);
    qDebug() << reply->errorString();
    reply->deleteLater();
}

void FlickrAPI::replyAuthError() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->disconnect(this);
    qDebug() << reply->errorString();
    reply->deleteLater();
    emit authResult(false);
}

//----------------------------------------------------------------------------------

quint64 FlickrAPI::timestamp() const {
    return QDateTime::currentMSecsSinceEpoch() / 1000;
}

QString FlickrAPI::nonce() const {
    return QString::number(qrand() % timestamp() + 1);
//    QCryptographicHash md5(QCryptographicHash::Md5);
//    md5.addData(QString::number(timestamp()).toAscii());
//    return md5.result().toBase64().constData();
}

//----------------------------------------------------------------------------------

inline QString FlickrAPI::extractReplyID(QObject *rawReply, QMap<QNetworkReply *, QString> &fromMap) {
    QNetworkReply *nwreply = qobject_cast<QNetworkReply*>(rawReply);
    QString id = fromMap[nwreply];
    fromMap.remove(nwreply);
    return id;
}

QByteArray FlickrAPI::getReplyContent(QObject *rawReply) const {
    QNetworkReply *qreply = qobject_cast<QNetworkReply*>(rawReply);
    if(!qreply) return QByteArray();
    QByteArray reply = qreply->readAll();
    qreply->disconnect(this);
    qreply->deleteLater();
    return reply;
}

QMap<QString, QString> FlickrAPI::parseReply(const QString &reply) const {
    QMap<QString, QString> res;
    QStringList tokens = reply.split('&');
    for(QStringList::Iterator t = tokens.begin(); t != tokens.end(); ++t) {
        QStringList keyValue = t->split('=');
        res[keyValue.at(0)] = keyValue.at(1);
    }
    return res;
}

//----------------------------------------------------------------------------------

QString FlickrAPI::baseStringFromURL(const QString &method, const QMap<QString, QString> &params, const QString &httpMethod) const {
    QString urlString;
    for(QMap<QString, QString>::ConstIterator p = params.begin(); p != params.end(); ++p) {
        urlString += "&" + p.key() + "=" + p.value();
    }
    urlString.remove(0, 1);

    QString encMethod = QUrl::toPercentEncoding(method).constData();
    QString encParams = QUrl::toPercentEncoding(urlString).constData();
    QString baseString = httpMethod + "&" + encMethod + "&" + encParams;
    return baseString;
}

QString FlickrAPI::urlFromParams(const QString &url, const QMap<QString, QString> &params) const {
    QString urlString = url + "?";
    for(QMap<QString, QString>::ConstIterator p = params.begin(); p != params.end(); ++p) {
        if(!p.key().startsWith("oauth")) urlString += p.key() + "=" + p.value() + "&";
    }
    return urlString.left(urlString.size() - 1);
}

QString FlickrAPI::hmacSha1(QByteArray key, const QByteArray &baseString) const {
    int blockSize = 64;
    if (key.length() > blockSize) {
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }

    QByteArray innerPadding(blockSize, char(0x36));
    QByteArray outerPadding(blockSize, char(0x5c));

    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i);
        outerPadding[i] = outerPadding[i] ^ key.at(i);
    }

    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}

//----------------------------------------------------------------------------------

QList<BigFileDescription> FlickrAPI::processFiles(QList<FileDescription> &flist) {
    qSort(flist.begin(), flist.end());
    QMap<QString, BigFileDescription> bigFileList;
    for(QList<FileDescription>::Iterator f = flist.begin(); f != flist.end(); ++f) {
        bigFileList[f->getCroppedName()].append(*f);
    }
    return bigFileList.values();
}

void FlickrAPI::emitDownloadPorgress(qint64 bd, qint64 bt) {
    QString fileName = fileDownloadReplyMap[qobject_cast<QNetworkReply*>(sender())];
    emit downloadProgress(bd, bt, fileName);
}
