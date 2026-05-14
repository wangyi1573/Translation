#pragma once
#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkReply>

class QNetworkAccessManager;

/**
 * @brief 火山引擎翻译API客户端
 * 使用HMAC-SHA256 V4签名
 */
class VolcengineApi : public QObject {
    Q_OBJECT

public:
    explicit VolcengineApi(QObject* parent = nullptr);
    ~VolcengineApi();

    void setCredentials(const QString& accessKeyId, const QString& accessKeySecret);
    void setRegion(const QString& region);

    void translate(const QString& text, const QString& sourceLang, const QString& targetLang);
    void detectLanguage(const QString& text);
    void testConnection();
    void cancelAll();

signals:
    void translateFinished(bool success, const QString& translated,
                         const QString& sourceLang, const QString& error);
    void detectFinished(bool success, const QString& langCode, const QString& error);
    void connectionTestFinished(bool success, const QString& error);

private slots:
    void onReplyFinished();

private:
    void sendSignedRequest(const QString& url, const QByteArray& body);
    void parseTranslateResponse(const QByteArray& data);
    void parseDetectResponse(const QByteArray& data);
    QString generateAuthorization(const QString& method, const QString& canonicalUri,
                                 const QMap<QString, QString>& headers, const QByteArray& body);
    QString sha256Hex(const QByteArray& data);
    QString hmacSha256Hex(const QString& key, const QString& data);
    QString getCurrentTimeISO8601();
    QString getCurrentDate();

    QNetworkAccessManager* m_nam;
    QNetworkReply* m_currentReply;
    QString m_accessKeyId;
    QString m_accessKeySecret;
    QString m_region;
    QString m_pendingAction;
    QString m_pendingSourceLang;
    QString m_pendingTargetLang;
};