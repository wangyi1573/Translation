#include "ApiClient.h"
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>

ApiClient::ApiClient(const QString& accessKeyId, const QString& accessKeySecret, QObject* parent)
    : QObject(parent),
      m_timeout(10000), // 默认10秒超时
      m_nextRequestId(1),
      m_maxRetries(3) // 默认最大重试3次
{
    m_networkManager = new QNetworkAccessManager(this);
    m_signatureGenerator = new SignatureGenerator(accessKeyId, accessKeySecret);
    
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &ApiClient::onNetworkReplyFinished);
}

ApiClient::~ApiClient()
{
    cancelAllRequests();
    // m_networkManager和m_signatureGenerator设置了parent为this，由Qt对象树自动管理，无需手动删除
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
    
    // 保存请求信息用于重试
    m_requestMethodMap[requestId] = method;
    m_requestUrlMap[requestId] = url;
    m_requestHeadersMap[requestId] = preparedHeaders;
    m_requestBodyMap[requestId] = body;
    m_retryCountMap[requestId] = 0;
    
    // 日志：输出请求信息
    qDebug() << "[ApiClient] Request" << requestId << ":" << method << url;
    qDebug() << "[ApiClient] Headers:" << preparedHeaders;
    if (!body.isEmpty()) {
        qDebug() << "[ApiClient] Body:" << QString::fromUtf8(body);
    }
    
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
        qDebug() << "[ApiClient] Request" << requestId << "finished successfully, response:" << QString::fromUtf8(response);
    } else {
        error = reply->errorString();
        success = false;
        qDebug() << "[ApiClient] Request" << requestId << "failed:" << error;
    }
    
    // 如果请求失败，检查是否需要重试
    if (!success) {
        int retryCount = m_retryCountMap.value(requestId, 0);
        if (retryCount < m_maxRetries) {
            bool shouldRetry = false;
            QNetworkReply::NetworkError netError = reply->error();
            
            // 网络错误或超时
            if (netError != QNetworkReply::NoError) {
                shouldRetry = true;
            } else {
                // 检查HTTP状态码
                int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                if (httpStatus >= 500 || httpStatus == 429) {
                    shouldRetry = true;
                }
            }
            
            if (shouldRetry) {
                qDebug() << "[ApiClient] Request" << requestId << "failed, retrying (" << retryCount + 1 << "/" << m_maxRetries << ")";
                int newRequestId = retryRequest(requestId);
                if (newRequestId != -1) {
                    // 重试成功发起，清理旧请求资源，不发射错误信号
                    return;
                }
            }
        }
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
        
        qDebug() << "[ApiClient] Request" << requestId << "timeout after" << m_timeout << "ms";
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
    
    // 清理重试相关映射
    m_retryCountMap.remove(requestId);
    m_requestMethodMap.remove(requestId);
    m_requestUrlMap.remove(requestId);
    m_requestHeadersMap.remove(requestId);
    m_requestBodyMap.remove(requestId);
}

int ApiClient::retryRequest(int oldRequestId)
{
    // 检查是否有旧的请求信息
    if (!m_requestMethodMap.contains(oldRequestId) || 
        !m_requestUrlMap.contains(oldRequestId) ||
        !m_requestHeadersMap.contains(oldRequestId)) {
        return -1;
    }
    
    // 获取旧的请求信息
    QString method = m_requestMethodMap[oldRequestId];
    QString url = m_requestUrlMap[oldRequestId];
    QMap<QString, QString> headers = m_requestHeadersMap[oldRequestId];
    QByteArray body = m_requestBodyMap[oldRequestId];
    int retryCount = m_retryCountMap.value(oldRequestId, 0) + 1;
    
    // 清理旧的请求资源（不包括映射，因为要复用）
    if (m_requests.contains(oldRequestId)) {
        QNetworkReply* oldReply = m_requests[oldRequestId];
        oldReply->abort();
        oldReply->deleteLater();
        m_requests.remove(oldRequestId);
    }
    if (m_timeoutTimers.contains(oldRequestId)) {
        QTimer* oldTimer = m_timeoutTimers[oldRequestId];
        oldTimer->stop();
        oldTimer->deleteLater();
        m_timeoutTimers.remove(oldRequestId);
    }
    
    // 创建新的请求ID
    int newRequestId = m_nextRequestId++;
    
    // 创建新的网络请求
    QUrl requestUrl(url);
    QNetworkRequest request;
    request.setUrl(requestUrl);
    
    // 设置请求头
    foreach (const QString& headerName, headers.keys()) {
        request.setRawHeader(headerName.toUtf8(), headers[headerName].toUtf8());
    }
    
    // 发送请求
    QNetworkReply* reply = nullptr;
    if (method.toUpper() == "POST") {
        reply = m_networkManager->post(request, body);
    } else {
        reply = m_networkManager->get(request);
    }
    
    // 更新映射
    m_requests[newRequestId] = reply;
    m_requestMethodMap[newRequestId] = method;
    m_requestUrlMap[newRequestId] = url;
    m_requestHeadersMap[newRequestId] = headers;
    m_requestBodyMap[newRequestId] = body;
    m_retryCountMap[newRequestId] = retryCount;
    
    // 设置新的超时定时器
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(m_timeout);
    connect(timer, &QTimer::timeout, [this, newRequestId]() {
        onRequestTimeout(newRequestId);
    });
    timer->start();
    m_timeoutTimers[newRequestId] = timer;
    
    // 清理旧的映射（已经转移到新的）
    m_retryCountMap.remove(oldRequestId);
    m_requestMethodMap.remove(oldRequestId);
    m_requestUrlMap.remove(oldRequestId);
    m_requestHeadersMap.remove(oldRequestId);
    m_requestBodyMap.remove(oldRequestId);
    
    qDebug() << "[ApiClient] Retrying request" << oldRequestId << "as" << newRequestId 
             << "(attempt" << retryCount << "of" << m_maxRetries << ")";
    
    return newRequestId;
}