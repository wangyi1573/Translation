#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QUrl>

/**
 * @brief HTTP客户端类
 * 负责与火山翻译API进行HTTP通信
 */
class HttpClient : public QObject {
    Q_OBJECT

public:
    HttpClient(QObject* parent = nullptr);
    ~HttpClient();
    
    void setTimeout(int timeout);
    
    int post(const QString& url, const QMap<QString, QString>& headers, const QByteArray& body);
    int get(const QString& url, const QMap<QString, QString>& headers);
    
    void cancelAllRequests();
    void cancelRequest(int requestId);

signals:
    void requestFinished(int requestId, bool success, const QByteArray& response, const QString& error);

private slots:
    void onNetworkReplyFinished();
    void onRequestTimeout();

private:
    int sendRequest(const QString& method, const QString& url,
                   const QMap<QString, QString>& headers, const QByteArray& body);
    
    int addRequest(int requestId, QNetworkReply* reply,
                   const QString& method, const QString& url,
                   const QMap<QString, QString>& headers, const QByteArray& body);
    
    void cleanupRequest(int requestId);
    
    QNetworkAccessManager* m_networkManager;
    int m_timeout;
    int m_nextRequestId;
    int m_maxRetries;
    
    struct RequestInfo {
        QNetworkReply* reply;
        QString method;
        QString url;
        QMap<QString, QString> headers;
        QByteArray body;
        int retryCount;
        QTimer* timer;
    };
    
    QMap<int, RequestInfo> m_requests;
};

#endif // HTTP_CLIENT_H