#ifndef TRANSLATIONCORE_H
#define TRANSLATIONCORE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QByteArray>

#include "ApiClient.h"
#include "TranslationResult.h"

/**
 * @brief 翻译核心类
 * 负责与火山翻译API交互，处理翻译请求和响应
 */
class TranslationCore : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param accessKeyId Access Key ID
     * @param accessKeySecret Access Key Secret
     * @param parent 父对象
     */
    TranslationCore(const QString& accessKeyId, const QString& accessKeySecret, QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~TranslationCore();
    
    /**
     * @brief 设置API密钥
     * @param accessKeyId Access Key ID
     * @param accessKeySecret Access Key Secret
     */
    void setApiKeys(const QString& accessKeyId, const QString& accessKeySecret);
    
    /**
     * @brief 设置请求超时时间
     * @param timeout 超时时间（毫秒）
     */
    void setTimeout(int timeout);
    
    /**
     * @brief 翻译单条文本
     * @param text 待翻译的文本
     * @param sourceLanguage 源语言代码，空字符串表示自动检测
     * @param targetLanguage 目标语言代码
     * @return 翻译请求ID
     */
    int translateText(const QString& text, const QString& sourceLanguage = "", const QString& targetLanguage = "zh");
    
    /**
     * @brief 翻译多条文本
     * @param texts 待翻译的文本列表
     * @param sourceLanguage 源语言代码，空字符串表示自动检测
     * @param targetLanguage 目标语言代码
     * @return 翻译请求ID
     */
    int translateTexts(const QStringList& texts, const QString& sourceLanguage = "", const QString& targetLanguage = "zh");
    
    /**
     * @brief 检测语言
     * @param text 待检测的文本
     * @return 语言检测请求ID
     */
    int detectLanguage(const QString& text);
    
    /**
     * @brief 测试API连接
     * @return 连接测试请求ID
     */
    int testConnection();
    
    /**
     * @brief 获取支持的语言列表
     * @return 语言代码到语言名称的映射
     */
    static QMap<QString, QString> getSupportedLanguages();

signals:
    /**
     * @brief 翻译完成信号
     * @param requestId 请求ID
     * @param result 翻译结果
     */
    void translationFinished(int requestId, const TranslationResult& result);
    
    /**
     * @brief 语言检测完成信号
     * @param requestId 请求ID
     * @param languageCode 检测到的语言代码
     * @param confidence 置信度
     * @param error 错误信息（如果有）
     */
    void languageDetectionFinished(int requestId, const QString& languageCode, float confidence, const QString& error);
    
    /**
     * @brief 连接测试完成信号
     * @param success 是否成功
     * @param message 测试结果消息
     */
    void connectionTestFinished(bool success, const QString& message);

private slots:
    /**
     * @brief API请求完成处理
     * @param requestId 请求ID
     * @param success 是否成功
     * @param response 响应数据
     * @param error 错误信息
     */
    void onApiRequestFinished(int requestId, bool success, const QByteArray& response, const QString& error);

private:
    ApiClient* m_apiClient;                 // API客户端
    QString m_accessKeyId;                  // Access Key ID
    QString m_accessKeySecret;              // Access Key Secret
    QMap<int, QString> m_requestTypeMap;    // 请求ID到请求类型的映射
    QMap<int, QStringList> m_requestTexts;  // 请求ID到请求文本列表的映射
    
    /**
     * @brief 构建翻译请求体
     * @param texts 待翻译的文本列表
     * @param sourceLanguage 源语言代码
     * @param targetLanguage 目标语言代码
     * @return 请求体JSON字符串
     */
    QByteArray buildTranslateRequestBody(const QStringList& texts, const QString& sourceLanguage, const QString& targetLanguage);
    
    /**
     * @brief 构建语言检测请求体
     * @param text 待检测的文本
     * @return 请求体JSON字符串
     */
    QByteArray buildLanguageDetectionRequestBody(const QString& text);
    
    /**
     * @brief 解析翻译响应
     * @param response 响应数据
     * @param requestId 请求ID
     * @return 翻译结果
     */
    TranslationResult parseTranslateResponse(const QByteArray& response, int requestId);
    
    /**
     * @brief 解析语言检测响应
     * @param response 响应数据
     * @param languageCode 检测到的语言代码
     * @param confidence 置信度
     * @param error 错误信息
     * @return 是否解析成功
     */
    bool parseLanguageDetectionResponse(const QByteArray& response, QString& languageCode, float& confidence, QString& error);
    
    /**
     * @brief 处理API错误
     * @param errorCode 错误代码
     * @param errorMessage 错误信息
     * @return 格式化的错误消息
     */
    QString handleApiError(int errorCode, const QString& errorMessage);
};

#endif // TRANSLATIONCORE_H