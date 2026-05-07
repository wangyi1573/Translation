#ifndef SIGNATURE_GENERATOR_H
#define SIGNATURE_GENERATOR_H

#include <QString>
#include <QByteArray>
#include <QMap>

/**
 * @brief 签名生成器类
 * 负责生成火山翻译API所需的HMAC-SHA256 V4签名
 */
class SignatureGenerator {
public:
    SignatureGenerator(const QString& accessKeyId, const QString& accessKeySecret,
                      const QString& region = "cn-north-1",
                      const QString& service = "translate");
    
    QString generateSignature(const QString& method, const QString& uriPath,
                             const QMap<QString, QString>& queryParams,
                             const QMap<QString, QString>& headers,
                             const QByteArray& requestBody);
    
    QString generateAuthorizationHeader(const QString& method, const QString& uriPath,
                                       const QMap<QString, QString>& queryParams,
                                       const QMap<QString, QString>& headers,
                                       const QByteArray& requestBody);
    
    static QString getCurrentTimeISO8601();
    static QString getCurrentDate();

private:
    QString m_accessKeyId;
    QString m_accessKeySecret;
    QString m_region;
    QString m_service;
    
    QString canonicalizeQueryString(const QMap<QString, QString>& queryParams);
    QString canonicalizeHeaders(const QMap<QString, QString>& headers,
                               const QStringList& signedHeaders);
    QString createCanonicalRequest(const QString& method, const QString& uriPath,
                                  const QString& canonicalQueryString,
                                  const QString& canonicalHeaders,
                                  const QStringList& signedHeaders,
                                  const QString& hashedPayload);
    QString createStringToSign(const QString& algorithm, const QString& requestDate,
                              const QString& credentialScope,
                              const QString& hashedCanonicalRequest);
    QByteArray deriveSigningKey(const QString& date);
    QByteArray hmacSha256(const QByteArray& key, const QString& data);
    QString sha256(const QByteArray& data);
    QString urlEncode(const QString& value);
};

#endif // SIGNATURE_GENERATOR_H