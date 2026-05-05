#include "ApiClient.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>

ApiClient::ApiClient(const QString& accessKeyId, const QString& accessKeySecret, QObject* parent)
    : QObject(parent),
      m_timeout(10000), // 默认10秒超时
      m_nextRequestId(1)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_signatureGenerator = new SignatureGenerator(accessKeyId, accessKeySecret);
    
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &ApiClient::onNetworkReplyFinished);
}

ApiClient::~ApiClient()
{
    cancelAllRequests();
    delete m_networkManager;
    delete m_signatureGenerator;
}

void ApiClient::setTimeout(int timeout)
{
    m_timeout = timeout;
}

int ApiClient::post(const QString& url, const QMap<QString, QString>& headers, const QByteArray& body)
{
    return sendRequest("POST", url, headers, body);
}

int ApiClient::get(const QString& url, const QMap<QString, QString>& headers)
{
    return sendRequest("GET", url, headers, QByteArray());
}

void ApiClient::cancelAllRequests()
{
    QList<int> requestIds = m_requests.keys();
    foreach (int requestId, requestIds) {
        cancelRequest(requestId);
    }
}

void ApiClient::cancelRequest(int requestId)
{
    if (m_requests.contains(requestId)) {
        QNetworkReply* reply = m_requests[requestId];
        reply->abort();
        reply->deleteLater();
        m_requests.remove(requestId);
        
        if (m_timeoutTimers.contains(requestId)) {
            QTimer* timer = m_timeoutTimers[requestId];
            timer->stop();
            timer->deleteLater();
            m_timeoutTimers.remove(requestId);
        }
    }
}

int ApiClient::sendRequest(const QString& method, const QString& url,
                          const QMap<QString, QString>& headers, const QByteArray& body)
{
    // 获取请求ID
    int requestId = m_nextRequestId++;
    
    // 解析URL
    QUrl requestUrl(url);
    QString host = requestUrl.host();
    QString path = requestUrl.path();
    if (path.isEmpty()) {
        path = "/";
    }
    
    // 提取查询参数
    QMap<QString, QString> queryParams;
    QUrlQuery query(requestUrl);
    foreach (const QString& key, query.keys()) {
        queryParams[key] = query.queryItemValue(key);
    }
    
    // 准备请求头
    QMap<QString, QString> preparedHeaders = prepareHeaders(method, requestUrl, headers, body);
    
    // 创建网络请求
    QNetworkRequest request;
    request.setUrl(requestUrl);
    
    // 设置请求头
    foreach (const QString& headerName, preparedHeaders.keys()) {
        request.setRawHeader(headerName.toUtf8(), preparedHeaders[headerName].toUtf8());
    }
    
    // 发送请求
    QNetworkReply* reply = nullptr;
    if (method.toUpper() == "POST") {
        reply = m_networkManager->post(request, body);
    } else {
        reply = m_networkManager->get(request);
    }
    
    // 保存请求
    m_requests[requestId] = reply;
    
    // 设置超时定时器
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(m_timeout);
    connect(timer, &QTimer::timeout, [this, requestId]() {
        onRequestTimeout(requestId);
    });
    timer->start();
    m_timeoutTimers[requestId] = timer;
    
    return requestId;
}

QMap<QString, QString> ApiClient::prepareHeaders(const QString& method, const QUrl& url,
                                               const QMap<QString, QString>& headers,
                                               const QByteArray& body)
{
    QMap<QString, QString> preparedHeaders = headers;
    
    // 添加必要的请求头
    if (!preparedHeaders.contains("Host")) {
        preparedHeaders["Host"] = url.host();
    }
    
    if (!preparedHeaders.contains("Content-Type")) {
        preparedHeaders["Content-Type"] = "application/json";
    }
    
    if (!preparedHeaders.contains("X-Date")) {
        preparedHeaders["X-Date"] = SignatureGenerator::getCurrentTimeISO8601();
    }
    
    if (!preparedHeaders.contains("X-Content-SHA256")) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(body);
        preparedHeaders["X-Content-SHA256"] = QString(hash.result().toHex());
    }
    
    // 生成Authorization头
    if (!preparedHeaders.contains("Authorization")) {
        QString path = url.path();
        if (path.isEmpty()) {
            path = "/";
        }
        
        QMap<QString, QString> queryParams;
        QUrlQuery query(url);
        foreach (const QString& key, query.keys()) {
            queryParams[key] = query.queryItemValue(key);
        }
        
        QString authorization = m_signatureGenerator->generateAuthorizationHeader(
            method, path, queryParams, preparedHeaders, body);
        preparedHeaders["Authorization"] = authorization;
    }
    
    return preparedHeaders;
}

void ApiClient::onNetworkReplyFinished(QNetworkReply* reply)
{
    // 查找请求ID
    int requestId = -1;
    QMap<int, QNetworkReply*>::iterator it;
    for (it = m_requests.begin(); it != m_requests.end(); ++it) {
        if (it.value() == reply) {
            requestId = it.key();
            break;
        }
    }
    
    if (requestId == -1) {
        reply->deleteLater();
        return;
    }
    
    // 清理超时定时器
    if (m_timeoutTimers.contains(requestId)) {
        QTimer* timer = m_timeoutTimers[requestId];
        timer->stop();
        timer->deleteLater();
        m_timeoutTimers.remove(requestId);
    }
    
    // 读取响应
    QByteArray response;
    QString error;
    bool success = false;
    
    if (reply->error() == QNetworkReply::NoError) {
        response = reply->readAll();
        success = true;
    } else {
        error = reply->errorString();
        success = false;
    }
    
    // 清理请求
    m_requests.remove(requestId);
    reply->deleteLater();
    
    // 发送信号
    emit requestFinished(requestId, success, response, error);
}

void ApiClient::onRequestTimeout(int requestId)
{
    if (m_requests.contains(requestId)) {
        QNetworkReply* reply = m_requests[requestId];
        reply->abort();
        reply->deleteLater();
        m_requests.remove(requestId);
        
        if (m_timeoutTimers.contains(requestId)) {
            QTimer* timer = m_timeoutTimers[requestId];
            timer->deleteLater();
            m_timeoutTimers.remove(requestId);
        }
        
        emit requestFinished(requestId, false, QByteArray(), "Request timeout");
    }
}

void ApiClient::cleanupRequest(int requestId)
{
    if (m_requests.contains(requestId)) {
        QNetworkReply* reply = m_requests[requestId];
        reply->deleteLater();
        m_requests.remove(requestId);
    }
    
    if (m_timeoutTimers.contains(requestId)) {
        QTimer* timer = m_timeoutTimers[requestId];
        timer->deleteLater();
        m_timeoutTimers.remove(requestId);
    }
}