#include "../include/PluginCore/SignatureGenerator.h"
#include <QCryptographicHash>
#include <QUrl>
#include <QUrlQuery>
#include <QStringList>
#include <algorithm>

SignatureGenerator::SignatureGenerator(const QString& accessKeyId, const QString& accessKeySecret,
                                     const QString& region, const QString& service)
    : m_accessKeyId(accessKeyId),
      m_accessKeySecret(accessKeySecret),
      m_region(region),
      m_service(service)
{
}

QString SignatureGenerator::generateSignature(const QString& method, const QString& uriPath,
                                            const QMap<QString, QString>& queryParams,
                                            const QMap<QString, QString>& headers,
                                            const QByteArray& requestBody)
{
    QString requestDate = getCurrentTimeISO8601();
    QString date = getCurrentDate();
    
    QString canonicalQueryString = canonicalizeQueryString(queryParams);
    
    QStringList signedHeaders;
    signedHeaders << "content-type" << "host" << "x-content-sha256" << "x-date";
    
    QString canonicalHeaders = canonicalizeHeaders(headers, signedHeaders);
    QString hashedPayload = sha256(requestBody);
    
    QString canonicalRequest = createCanonicalRequest(method, uriPath, canonicalQueryString,
                                                    canonicalHeaders, signedHeaders, hashedPayload);
    QString hashedCanonicalRequest = sha256(canonicalRequest.toUtf8());
    
    QString credentialScope = date + "/" + m_region + "/" + m_service + "/request";
    QString stringToSign = createStringToSign("HMAC-SHA256", requestDate, credentialScope, hashedCanonicalRequest);
    
    QByteArray signingKey = deriveSigningKey(date);
    QByteArray signature = hmacSha256(signingKey, stringToSign);
    
    return QString(signature.toHex());
}

QString SignatureGenerator::generateAuthorizationHeader(const QString& method, const QString& uriPath,
                                                      const QMap<QString, QString>& queryParams,
                                                      const QMap<QString, QString>& headers,
                                                      const QByteArray& requestBody)
{
    QString date = getCurrentDate();
    QString signature = generateSignature(method, uriPath, queryParams, headers, requestBody);
    QString credentialScope = date + "/" + m_region + "/" + m_service + "/request";
    
    return "HMAC-SHA256 "
           "Credential=" + m_accessKeyId + "/" + credentialScope + ", "
           "SignedHeaders=content-type;host;x-content-sha256;x-date, "
           "Signature=" + signature;
}

QString SignatureGenerator::getCurrentTimeISO8601()
{
    return QDateTime::currentDateTimeUtc().toString("yyyyMMddTHHmmssZ");
}

QString SignatureGenerator::getCurrentDate()
{
    return QDateTime::currentDateTimeUtc().toString("yyyyMMdd");
}

QString SignatureGenerator::canonicalizeQueryString(const QMap<QString, QString>& queryParams)
{
    if (queryParams.isEmpty()) {
        return "";
    }
    
    QStringList paramList;
    QList<QString> sortedKeys = queryParams.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());
    
    for (const QString& key : sortedKeys) {
        QString value = queryParams.value(key);
        paramList << urlEncode(key) + "=" + urlEncode(value);
    }
    
    return paramList.join("&");
}

QString SignatureGenerator::canonicalizeHeaders(const QMap<QString, QString>& headers,
                                              const QStringList& signedHeaders)
{
    QStringList headerList;
    QStringList sortedHeaders = signedHeaders;
    std::sort(sortedHeaders.begin(), sortedHeaders.end());
    
    for (const QString& headerName : sortedHeaders) {
        QString lowerHeaderName = headerName.toLower();
        QString headerValue = headers.value(lowerHeaderName, "");
        headerValue = headerValue.trimmed();
        headerValue.replace(QRegExp("\\s+"), " ");
        headerList << lowerHeaderName + ":" + headerValue;
    }
    
    return headerList.join("\n") + "\n";
}

QString SignatureGenerator::createCanonicalRequest(const QString& method, const QString& uriPath,
                                                 const QString& canonicalQueryString,
                                                 const QString& canonicalHeaders,
                                                 const QStringList& signedHeaders,
                                                 const QString& hashedPayload)
{
    return method.toUpper() + "\n"
         + uriPath + "\n"
         + canonicalQueryString + "\n"
         + canonicalHeaders + "\n"
         + signedHeaders.join(";") + "\n"
         + hashedPayload;
}

QString SignatureGenerator::createStringToSign(const QString& algorithm, const QString& requestDate,
                                             const QString& credentialScope,
                                             const QString& hashedCanonicalRequest)
{
    return algorithm + "\n"
         + requestDate + "\n"
         + credentialScope + "\n"
         + hashedCanonicalRequest;
}

QByteArray SignatureGenerator::deriveSigningKey(const QString& date)
{
    QByteArray kDate = hmacSha256("AWS4" + m_accessKeySecret.toUtf8(), date);
    QByteArray kRegion = hmacSha256(kDate, m_region);
    QByteArray kService = hmacSha256(kRegion, m_service);
    return hmacSha256(kService, "request");
}

QByteArray SignatureGenerator::hmacSha256(const QByteArray& key, const QString& data)
{
    QByteArray k_ipad(64, 0);
    QByteArray k_opad(64, 0);
    
    QByteArray keyData = key;
    if (keyData.length() > 64) {
        keyData = QCryptographicHash::hash(keyData, QCryptographicHash::Sha256);
    }
    
    for (int i = 0; i < 64; ++i) {
        k_ipad[i] = (i < keyData.length() ? keyData[i] : 0) ^ 0x36;
        k_opad[i] = (i < keyData.length() ? keyData[i] : 0) ^ 0x5c;
    }
    
    QByteArray innerData = k_ipad + data.toUtf8();
    QByteArray innerHash = QCryptographicHash::hash(innerData, QCryptographicHash::Sha256);
    
    QByteArray outerData = k_opad + innerHash;
    return QCryptographicHash::hash(outerData, QCryptographicHash::Sha256);
}

QString SignatureGenerator::sha256(const QByteArray& data)
{
    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

QString SignatureGenerator::urlEncode(const QString& value)
{
    QUrlQuery query;
    query.addQueryItem("dummy", value);
    QString encoded = query.toString(QUrl::FullyEncoded);
    return encoded.mid(7);
}