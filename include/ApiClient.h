#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QUrl>

#include "SignatureGenerator.h"

/**
 * @brief API客户端类
 * 负责与火山翻译API进行HTTP通信
 */
class ApiClient : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param accessKeyId Access Key ID
     * @param accessKeySecret Access Key Secret
     * @param parent 父对象
     */
    ApiClient(const QString& accessKeyId, const QString& accessKeySecret, QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ApiClient();
    
    /**
     * @brief 设置请求超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setTimeout(int timeout);
    
    /**
     * @brief 发送POST请求
     * @param url 请求URL
     * @param headers 请求头
     * @param body 请求体
     * @return 请求ID，用于标识此请求
     */
    int post(const QString& url, const QMap<QString, QString>& headers, const QByteArray& body);
    
    /**
     * @brief 发送GET请求
     * @param url 请求URL
     * @param headers 请求头
     * @return 请求ID，用于标识此请求
     */
    int get(const QString& url, const QMap<QString, QString>& headers);
    
    /**
     * @brief 取消所有请求
     */
    void cancelAllRequests();
    
    /**
     * @brief 取消指定ID的请求
     * @param requestId 请求ID
     */
    void cancelRequest(int requestId);

signals:
    /**
     * @brief 请求完成信号
     * @param requestId 请求ID
     * @param success 是否成功
     * @param response 响应数据
     * @param error 错误信息（如果有）
     */
    void requestFinished(int requestId, bool success, const QByteArray& response, const QString& error);

private slots:
    /**
     * @brief 网络请求完成处理
     * @param reply 网络回复
     */
    void onNetworkReplyFinished(QNetworkReply* reply);
    
    /**
     * @brief 请求超时处理
     * @param requestId 请求ID
     */
    void onRequestTimeout(int requestId);

private:
    QNetworkAccessManager* m_networkManager;  // 网络访问管理器
    SignatureGenerator* m_signatureGenerator; // 签名生成器
    int m_timeout;                           // 请求超时时间（毫秒）
    int m_nextRequestId;                     // 下一个请求ID
    QMap<int, QNetworkReply*> m_requests;    // 请求ID到网络回复的映射
    QMap<int, QTimer*> m_timeoutTimers;      // 请求ID到超时定时器的映射
    
    /**
     * @brief 发送请求
     * @param method HTTP方法
     * @param url 请求URL
     * @param headers 请求头
     * @param body 请求体
     * @return 请求ID
     */
    int sendRequest(const QString& method, const QString& url, 
                   const QMap<QString, QString>& headers, const QByteArray& body);
    
    /**
     * @brief 准备请求头
     * @param method HTTP方法
     * @param url 请求URL
     * @param headers 用户提供的请求头
     * @param body 请求体
     * @return 完整的请求头
     */
    QMap<QString, QString> prepareHeaders(const QString& method, const QUrl& url,
                                         const QMap<QString, QString>& headers,
                                         const QByteArray& body);
    
    /**
     * @brief 清理请求资源
     * @param requestId 请求ID
     */
    void cleanupRequest(int requestId);
};

#endif // APICLIENT_H