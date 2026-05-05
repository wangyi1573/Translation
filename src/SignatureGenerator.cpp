#include "SignatureGenerator.h"
#include <QCryptographicHash>
#include <QUrl>
#include <QStringList>
#include <QDebug>

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
    // 获取当前时间
    QString requestDate = getCurrentTimeISO8601();
    QString date = getCurrentDate();
    
    // 规范化查询字符串
    QString canonicalQueryString = canonicalizeQueryString(queryParams);
    
    // 准备要签名的请求头
    QStringList signedHeaders;
    signedHeaders << "content-type" << "host" << "x-content-sha256" << "x-date";
    
    // 规范化请求头
    QString canonicalHeaders = canonicalizeHeaders(headers, signedHeaders);
    
    // 计算请求体的哈希值
    QString hashedPayload = sha256(requestBody);
    
    // 创建规范化请求
    QString canonicalRequest = createCanonicalRequest(method, uriPath, canonicalQueryString,
                                                    canonicalHeaders, signedHeaders, hashedPayload);
    
    // 计算规范化请求的哈希值
    QString hashedCanonicalRequest = sha256(canonicalRequest.toUtf8());
    
    // 创建凭证范围
    QString credentialScope = date + "/" + m_region + "/" + m_service + "/request";
    
    // 创建待签字符串
    QString stringToSign = createStringToSign("HMAC-SHA256", requestDate, credentialScope, hashedCanonicalRequest);
    
    // 派生签名密钥
    QByteArray signingKey = deriveSigningKey(date);
    
    // 计算最终签名
    QByteArray signature = hmacSha256(signingKey, stringToSign);
    
    // 返回十六进制格式的签名
    return QString(signature.toHex());
}

QString SignatureGenerator::generateAuthorizationHeader(const QString& method, const QString& uriPath,
                                                      const QMap<QString, QString>& queryParams,
                                                      const QMap<QString, QString>& headers,
                                                      const QByteArray& requestBody)
{
    // 获取当前时间
    QString date = getCurrentDate();
    
    // 生成签名
    QString signature = generateSignature(method, uriPath, queryParams, headers, requestBody);
    
    // 创建凭证范围
    QString credentialScope = date + "/" + m_region + "/" + m_service + "/request";
    
    // 创建Authorization头
    QString authorization = "HMAC-SHA256 "
                          "Credential=" + m_accessKeyId + "/" + credentialScope + ", "
                          "SignedHeaders=content-type;host;x-content-sha256;x-date, "
                          "Signature=" + signature;
    
    return authorization;
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
    
    // 对查询参数进行排序
    QList<QString> sortedKeys = queryParams.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());
    
    // 构建规范化的查询字符串
    foreach (const QString& key, sortedKeys) {
        QString value = queryParams.value(key);
        paramList << urlEncode(key) + "=" + urlEncode(value);
    }
    
    return paramList.join("&");
}

QString SignatureGenerator::canonicalizeHeaders(const QMap<QString, QString>& headers,
                                              const QStringList& signedHeaders)
{
    QStringList headerList;
    
    // 对请求头进行排序
    QStringList sortedHeaders = signedHeaders;
    std::sort(sortedHeaders.begin(), sortedHeaders.end());
    
    // 构建规范化的请求头字符串
    foreach (const QString& headerName, sortedHeaders) {
        QString lowerHeaderName = headerName.toLower();
        QString headerValue = headers.value(lowerHeaderName, "");
        
        // 移除首尾空白字符并将连续的空白字符替换为单个空格
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
    QString canonicalRequest = method.toUpper() + "\n"
                             + uriPath + "\n"
                             + canonicalQueryString + "\n"
                             + canonicalHeaders + "\n"
                             + signedHeaders.join(";") + "\n"
                             + hashedPayload;
    
    return canonicalRequest;
}

QString SignatureGenerator::createStringToSign(const QString& algorithm, const QString& requestDate,
                                             const QString& credentialScope,
                                             const QString& hashedCanonicalRequest)
{
    QString stringToSign = algorithm + "\n"
                         + requestDate + "\n"
                         + credentialScope + "\n"
                         + hashedCanonicalRequest;
    
    return stringToSign;
}

QByteArray SignatureGenerator::deriveSigningKey(const QString& date)
{
    // 步骤1: HMAC-SHA256(date, "AWS4" + secretKey)
    QByteArray kDate = hmacSha256("AWS4" + m_accessKeySecret.toUtf8(), date);
    
    // 步骤2: HMAC-SHA256(region, kDate)
    QByteArray kRegion = hmacSha256(kDate, m_region);
    
    // 步骤3: HMAC-SHA256(service, kRegion)
    QByteArray kService = hmacSha256(kRegion, m_service);
    
    // 步骤4: HMAC-SHA256("request", kService)
    QByteArray kSigning = hmacSha256(kService, "request");
    
    return kSigning;
}

QByteArray SignatureGenerator::hmacSha256(const QByteArray& key, const QString& data)
{
    return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);
}

QString SignatureGenerator::sha256(const QByteArray& data)
{
    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

QString SignatureGenerator::urlEncode(const QString& value)
{
    QUrl url;
    url.setQueryItem("param", value);
    QString encoded = url.query().mid(6); // 移除"param="前缀
    
    // 替换不符合规范的字符
    encoded.replace("+", "%20");
    encoded.replace("*", "%2A");
    encoded.replace("%7E", "~");
    
    return encoded;
}