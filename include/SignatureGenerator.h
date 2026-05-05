#ifndef SIGNATUREGENERATOR_H
#define SIGNATUREGENERATOR_H

#include <QString>
#include <QByteArray>
#include <QMap>
#include <QDateTime>

/**
 * @brief 签名生成器类
 * 负责生成火山翻译API所需的HMAC-SHA256 V4签名
 */
class SignatureGenerator {
public:
    /**
     * @brief 构造函数
     * @param accessKeyId Access Key ID
     * @param accessKeySecret Access Key Secret
     * @param region 区域，默认为"cn-north-1"
     * @param service 服务名，默认为"translate"
     */
    SignatureGenerator(const QString& accessKeyId, const QString& accessKeySecret,
                      const QString& region = "cn-north-1",
                      const QString& service = "translate");
    
    /**
     * @brief 生成API请求签名
     * @param method HTTP方法（GET/POST）
     * @param uriPath URI路径，默认为"/"
     * @param queryParams 查询参数
     * @param headers 请求头
     * @param requestBody 请求体
     * @return 生成的签名
     */
    QString generateSignature(const QString& method, const QString& uriPath,
                             const QMap<QString, QString>& queryParams,
                             const QMap<QString, QString>& headers,
                             const QByteArray& requestBody);
    
    /**
     * @brief 生成Authorization请求头
     * @param method HTTP方法（GET/POST）
     * @param uriPath URI路径，默认为"/"
     * @param queryParams 查询参数
     * @param headers 请求头
     * @param requestBody 请求体
     * @return Authorization请求头值
     */
    QString generateAuthorizationHeader(const QString& method, const QString& uriPath,
                                       const QMap<QString, QString>& queryParams,
                                       const QMap<QString, QString>& headers,
                                       const QByteArray& requestBody);
    
    /**
     * @brief 获取当前时间的ISO8601格式字符串
     * @return ISO8601格式的时间字符串
     */
    static QString getCurrentTimeISO8601();
    
    /**
     * @brief 获取当前日期（YYYYMMDD格式）
     * @return YYYYMMDD格式的日期字符串
     */
    static QString getCurrentDate();

private:
    QString m_accessKeyId;      // Access Key ID
    QString m_accessKeySecret;  // Access Key Secret
    QString m_region;           // 区域
    QString m_service;          // 服务名
    
    /**
     * @brief 规范化查询字符串
     * @param queryParams 查询参数
     * @return 规范化的查询字符串
     */
    QString canonicalizeQueryString(const QMap<QString, QString>& queryParams);
    
    /**
     * @brief 规范化请求头
     * @param headers 请求头
     * @param signedHeaders 要签名的请求头列表
     * @return 规范化的请求头字符串
     */
    QString canonicalizeHeaders(const QMap<QString, QString>& headers,
                               const QStringList& signedHeaders);
    
    /**
     * @brief 创建规范化请求
     * @param method HTTP方法
     * @param uriPath URI路径
     * @param canonicalQueryString 规范化的查询字符串
     * @param canonicalHeaders 规范化的请求头
     * @param signedHeaders 要签名的请求头列表
     * @param hashedPayload 请求体的哈希值
     * @return 规范化请求字符串
     */
    QString createCanonicalRequest(const QString& method, const QString& uriPath,
                                  const QString& canonicalQueryString,
                                  const QString& canonicalHeaders,
                                  const QStringList& signedHeaders,
                                  const QString& hashedPayload);
    
    /**
     * @brief 创建待签字符串
     * @param algorithm 算法
     * @param requestDate 请求日期
     * @param credentialScope 凭证范围
     * @param hashedCanonicalRequest 规范化请求的哈希值
     * @return 待签字符串
     */
    QString createStringToSign(const QString& algorithm, const QString& requestDate,
                              const QString& credentialScope,
                              const QString& hashedCanonicalRequest);
    
    /**
     * @brief 派生签名密钥
     * @param date 日期（YYYYMMDD格式）
     * @return 签名密钥
     */
    QByteArray deriveSigningKey(const QString& date);
    
    /**
     * @brief 计算HMAC-SHA256哈希
     * @param key 密钥
     * @param data 数据
     * @return HMAC-SHA256哈希值
     */
    QByteArray hmacSha256(const QByteArray& key, const QString& data);
    
    /**
     * @brief 计算SHA256哈希
     * @param data 数据
     * @return SHA256哈希的十六进制字符串
     */
    QString sha256(const QByteArray& data);
    
    /**
     * @brief URL编码
     * @param value 要编码的值
     * @return URL编码后的值
     */
    QString urlEncode(const QString& value);
};

#endif // SIGNATUREGENERATOR_H