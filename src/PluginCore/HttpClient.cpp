#include "../include/PluginCore/HttpClient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

HttpClient::HttpClient(QObject* parent)
    : QObject(parent),
      m_networkManager(new QNetworkAccessManager(this)),
      m_timeout(30000),
      m_nextRequestId(1),
      m_maxRetries(3)
{
    // Connect signals
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &HttpClient::onNetworkReplyFinished);
}

HttpClient::~HttpClient()
{
    cancelAllRequests();
}

void HttpClient::setTimeout(int timeout)
{
    m_timeout = timeout;
}

int HttpClient::post(const QString& url, const QMap<QString, QString>& headers, const QByteArray& body)
{
    return sendRequest("POST", url, headers, body);
}

int HttpClient::get(const QString& url, const QMap<QString, QString>& headers)
{
    return sendRequest("GET", url, headers, QByteArray());
}

int HttpClient::sendRequest(const QString& method, const QString& url,
                           const QMap<QString, QString>& headers, const QByteArray& body)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Content-Type", "application/json");
    
    // Apply custom headers
    for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    QNetworkReply* reply = nullptr;
    if (method == "POST") {
        reply = m_networkManager->post(request, body);
    } else if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "DELETE") {
        reply = m_networkManager->deleteResource(request);
    }
    
    if (!reply) {
        return -1;
    }
    
    int requestId = m_nextRequestId++;
    addRequest(requestId, reply, method, url, headers, body);
    
    return requestId;
}

int HttpClient::addRequest(int requestId, QNetworkReply* reply,
                          const QString& method, const QString& url,
                          const QMap<QString, QString>& headers, const QByteArray& body)
{
    RequestInfo info;
    info.reply = reply;
    info.method = method;
    info.url = url;
    info.headers = headers;
    info.body = body;
    info.retryCount = 0;
    info.timer = new QTimer(this);
    info.timer->setSingleShot(true);
    connect(info.timer, &QTimer::timeout, this, &HttpClient::onRequestTimeout);
    info.timer->start(m_timeout);
    
    m_requests[requestId] = info;
    return requestId;
}

void HttpClient::onNetworkReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    // Find request ID
    int requestId = -1;
    for (auto it = m_requests.begin(); it != m_requests.end(); ++it) {
        if (it.value().reply == reply) {
            requestId = it.key();
            break;
        }
    }
    
    if (requestId == -1) {
        reply->deleteLater();
        return;
    }
    
    RequestInfo info = m_requests[requestId];
    m_requests.remove(requestId);
    
    if (info.timer) {
        info.timer->stop();
        delete info.timer;
    }
    
    bool success = false;
    QByteArray response;
    QString error;
    
    if (reply->error() == QNetworkReply::NoError) {
        response = reply->readAll();
        success = true;
    } else {
        error = reply->errorString();
        
        // Retry logic
        if (info.retryCount < m_maxRetries) {
            reply->deleteLater();
            info.retryCount++;
            QTimer::singleShot(1000, this, [this, info, requestId]() {
                // Re-send request
                QNetworkRequest request;
                request.setUrl(QUrl(info.url));
                request.setRawHeader("Content-Type", "application/json");
                for (auto it = info.headers.constBegin(); it != info.headers.constEnd(); ++it) {
                    request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
                }
                
                QNetworkReply* newReply = nullptr;
                if (info.method == "POST") {
                    newReply = m_networkManager->post(request, info.body);
                } else {
                    newReply = m_networkManager->get(request);
                }
                
                if (newReply) {
                    addRequest(requestId, newReply, info.method, info.url, info.headers, info.body);
                    m_requests[requestId].retryCount = info.retryCount;
                }
            });
            return;
        }
    }
    
    reply->deleteLater();
    emit requestFinished(requestId, success, response, error);
}

void HttpClient::onRequestTimeout()
{
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer) return;
    
    for (auto it = m_requests.begin(); it != m_requests.end(); ++it) {
        if (it.value().timer == timer) {
            int requestId = it.key();
            RequestInfo info = it.value();
            m_requests.remove(requestId);
            
            if (info.reply) {
                info.reply->abort();
                info.reply->deleteLater();
            }
            
            emit requestFinished(requestId, false, QByteArray(), "Request timeout");
            break;
        }
    }
}

void HttpClient::cancelAllRequests()
{
    for (auto it = m_requests.begin(); it != m_requests.end(); ++it) {
        RequestInfo& info = it.value();
        if (info.timer) {
            info.timer->stop();
            delete info.timer;
        }
        if (info.reply) {
            info.reply->abort();
            info.reply->deleteLater();
        }
    }
    m_requests.clear();
}

void HttpClient::cancelRequest(int requestId)
{
    if (m_requests.contains(requestId)) {
        RequestInfo& info = m_requests[requestId];
        if (info.timer) {
            info.timer->stop();
            delete info.timer;
        }
        if (info.reply) {
            info.reply->abort();
            info.reply->deleteLater();
        }
        m_requests.remove(requestId);
    }
}

void HttpClient::cleanupRequest(int requestId)
{
    if (m_requests.contains(requestId)) {
        RequestInfo& info = m_requests[requestId];
        if (info.timer) {
            info.timer->stop();
            delete info.timer;
        }
        if (info.reply) {
            info.reply->deleteLater();
        }
        m_requests.remove(requestId);
    }
}