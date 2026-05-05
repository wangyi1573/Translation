#include "TranslationCore.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

TranslationCore::TranslationCore(const QString& accessKeyId, const QString& accessKeySecret, QObject* parent)
    : QObject(parent),
      m_accessKeyId(accessKeyId),
      m_accessKeySecret(accessKeySecret)
{
    m_apiClient = new ApiClient(accessKeyId, accessKeySecret, this);
    connect(m_apiClient, &ApiClient::requestFinished, this, &TranslationCore::onApiRequestFinished);
}

TranslationCore::~TranslationCore()
{
    delete m_apiClient;
}

void TranslationCore::setApiKeys(const QString& accessKeyId, const QString& accessKeySecret)
{
    m_accessKeyId = accessKeyId;
    m_accessKeySecret = accessKeySecret;
    
    // 重新创建API客户端
    delete m_apiClient;
    m_apiClient = new ApiClient(accessKeyId, accessKeySecret, this);
    connect(m_apiClient, &ApiClient::requestFinished, this, &TranslationCore::onApiRequestFinished);
}

void TranslationCore::setTimeout(int timeout)
{
    m_apiClient->setTimeout(timeout);
}

int TranslationCore::translateText(const QString& text, const QString& sourceLanguage, const QString& targetLanguage)
{
    QStringList texts;
    texts << text;
    return translateTexts(texts, sourceLanguage, targetLanguage);
}

int TranslationCore::translateTexts(const QStringList& texts, const QString& sourceLanguage, const QString& targetLanguage)
{
    // 构建请求体
    QByteArray requestBody = buildTranslateRequestBody(texts, sourceLanguage, targetLanguage);
    
    // 设置请求头
    QMap<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    
    // 设置查询参数
    QMap<QString, QString> queryParams;
    queryParams["Action"] = "TranslateText";
    queryParams["Version"] = "2020-06-01";
    
    // 发送请求
    int requestId = m_apiClient->post("https://translate.volcengineapi.com", headers, requestBody);
    
    // 保存请求信息
    m_requestTypeMap[requestId] = "translate";
    m_requestTexts[requestId] = texts;
    
    return requestId;
}

int TranslationCore::detectLanguage(const QString& text)
{
    // 构建请求体
    QByteArray requestBody = buildLanguageDetectionRequestBody(text);
    
    // 设置请求头
    QMap<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    
    // 设置查询参数
    QMap<QString, QString> queryParams;
    queryParams["Action"] = "LangDetect";
    queryParams["Version"] = "2020-06-01";
    
    // 发送请求
    int requestId = m_apiClient->post("https://translate.volcengineapi.com", headers, requestBody);
    
    // 保存请求信息
    m_requestTypeMap[requestId] = "detect";
    
    return requestId;
}

int TranslationCore::testConnection()
{
    // 测试连接使用简单的翻译请求
    return translateText("Hello", "", "zh");
}

QMap<QString, QString> TranslationCore::getSupportedLanguages()
{
    QMap<QString, QString> languages;
    
    // 常用语言列表
    languages["auto"] = "自动检测";
    languages["zh"] = "中文";
    languages["en"] = "英文";
    languages["ja"] = "日语";
    languages["ko"] = "韩语";
    languages["fr"] = "法语";
    languages["de"] = "德语";
    languages["es"] = "西班牙语";
    languages["ru"] = "俄语";
    languages["pt"] = "葡萄牙语";
    languages["it"] = "意大利语";
    languages["nl"] = "荷兰语";
    languages["pl"] = "波兰语";
    languages["ar"] = "阿拉伯语";
    languages["tr"] = "土耳其语";
    languages["th"] = "泰语";
    languages["vi"] = "越南语";
    languages["id"] = "印尼语";
    languages["ms"] = "马来语";
    languages["hi"] = "印地语";
    
    return languages;
}

void TranslationCore::onApiRequestFinished(int requestId, bool success, const QByteArray& response, const QString& error)
{
    if (!m_requestTypeMap.contains(requestId)) {
        return;
    }
    
    QString requestType = m_requestTypeMap[requestId];
    
    if (requestType == "translate") {
        // 处理翻译响应
        TranslationResult result = parseTranslateResponse(response, requestId);
        
        // 如果是测试连接请求，发送连接测试完成信号
        if (m_requestTexts[requestId].size() == 1 && m_requestTexts[requestId].first() == "Hello") {
            bool testSuccess = result.success;
            QString message = testSuccess ? "连接成功" : result.errorMessage;
            emit connectionTestFinished(testSuccess, message);
        } else {
            // 否则发送翻译完成信号
            emit translationFinished(requestId, result);
        }
    } else if (requestType == "detect") {
        // 处理语言检测响应
        QString languageCode;
        float confidence = 0.0f;
        QString errorMessage;
        
        bool parseSuccess = parseLanguageDetectionResponse(response, languageCode, confidence, errorMessage);
        
        if (parseSuccess) {
            emit languageDetectionFinished(requestId, languageCode, confidence, "");
        } else {
            emit languageDetectionFinished(requestId, "", 0.0f, errorMessage);
        }
    }
    
    // 清理请求信息
    m_requestTypeMap.remove(requestId);
    m_requestTexts.remove(requestId);
}

QByteArray TranslationCore::buildTranslateRequestBody(const QStringList& texts, const QString& sourceLanguage, const QString& targetLanguage)
{
    QJsonObject requestBody;
    
    // 设置目标语言
    requestBody["TargetLanguage"] = targetLanguage;
    
    // 设置源语言（如果提供）
    if (!sourceLanguage.isEmpty() && sourceLanguage != "auto") {
        requestBody["SourceLanguage"] = sourceLanguage;
    }
    
    // 设置文本列表
    QJsonArray textArray;
    foreach (const QString& text, texts) {
        textArray.append(text);
    }
    requestBody["TextList"] = textArray;
    
    // 转换为JSON字符串
    QJsonDocument doc(requestBody);
    return doc.toJson(QJsonDocument::Compact);
}

QByteArray TranslationCore::buildLanguageDetectionRequestBody(const QString& text)
{
    QJsonObject requestBody;
    requestBody["Text"] = text;
    
    // 转换为JSON字符串
    QJsonDocument doc(requestBody);
    return doc.toJson(QJsonDocument::Compact);
}

TranslationResult TranslationCore::parseTranslateResponse(const QByteArray& response, int requestId)
{
    TranslationResult result;
    
    // 解析JSON响应
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (doc.isNull()) {
        result.success = false;
        result.errorCode = -1;
        result.errorMessage = "无效的JSON响应";
        return result;
    }
    
    QJsonObject rootObject = doc.object();
    
    // 检查是否有错误
    if (rootObject.contains("ResponseMetadata")) {
        QJsonObject responseMetadata = rootObject["ResponseMetadata"].toObject();
        if (responseMetadata.contains("Error")) {
            QJsonObject error = responseMetadata["Error"].toObject();
            result.success = false;
            result.errorCode = error["Code"].toString().toInt();
            result.errorMessage = error["Message"].toString();
            return result;
        }
    }
    
    // 解析翻译结果
    if (rootObject.contains("TranslationList")) {
        QJsonArray translationList = rootObject["TranslationList"].toArray();
        
        if (translationList.size() > 0) {
            QJsonObject translationObj = translationList[0].toObject();
            
            // 获取翻译文本
            if (translationObj.contains("Translation")) {
                result.translatedText = translationObj["Translation"].toString();
            }
            
            // 获取检测到的源语言（如果有）
            if (translationObj.contains("DetectedSourceLanguage")) {
                result.sourceLanguage = translationObj["DetectedSourceLanguage"].toString();
            }
            
            // 获取请求的目标语言
            if (rootObject.contains("TargetLanguage")) {
                result.targetLanguage = rootObject["TargetLanguage"].toString();
            }
            
            // 获取请求的源语言（如果有）
            if (rootObject.contains("SourceLanguage")) {
                result.sourceLanguage = rootObject["SourceLanguage"].toString();
            }
            
            result.success = true;
            result.errorCode = 0;
        } else {
            result.success = false;
            result.errorCode = -2;
            result.errorMessage = "翻译结果为空";
        }
    } else {
        result.success = false;
        result.errorCode = -3;
        result.errorMessage = "响应中没有翻译结果";
    }
    
    return result;
}

bool TranslationCore::parseLanguageDetectionResponse(const QByteArray& response, QString& languageCode, float& confidence, QString& error)
{
    // 解析JSON响应
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (doc.isNull()) {
        error = "无效的JSON响应";
        return false;
    }
    
    QJsonObject rootObject = doc.object();
    
    // 检查是否有错误
    if (rootObject.contains("ResponseMetadata")) {
        QJsonObject responseMetadata = rootObject["ResponseMetadata"].toObject();
        if (responseMetadata.contains("Error")) {
            QJsonObject errorObj = responseMetadata["Error"].toObject();
            error = errorObj["Message"].toString();
            return false;
        }
    }
    
    // 解析语言检测结果
    if (rootObject.contains("DetectedLanguage")) {
        languageCode = rootObject["DetectedLanguage"].toString();
        
        if (rootObject.contains("Confidence")) {
            confidence = rootObject["Confidence"].toDouble();
        } else {
            confidence = 1.0f; // 默认置信度
        }
        
        return true;
    } else {
        error = "响应中没有语言检测结果";
        return false;
    }
}

QString TranslationCore::handleApiError(int errorCode, const QString& errorMessage)
{
    QString message;
    
    switch (errorCode) {
    case -400:
        message = "请求参数错误: " + errorMessage;
        break;
    case -415:
        message = "不支持的翻译: " + errorMessage;
        break;
    case -429:
        message = "请求过于频繁，请稍后再试";
        break;
    case -500:
        message = "翻译服务内部错误: " + errorMessage;
        break;
    default:
        message = "翻译失败 (错误码: " + QString::number(errorCode) + "): " + errorMessage;
        break;
    }
    
    return message;
}