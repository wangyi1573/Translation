#include "TranslationCore.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

TranslationCore::TranslationCore(const QString& accessKeyId, const QString& accessKeySecret, QObject* parent)
    : QObject(parent),
      m_accessKeyId(accessKeyId),
      m_accessKeySecret(accessKeySecret),
      m_languagesFetched(false)
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
    // 检查缓存（未过期）
    QString cacheKey = generateCacheKey(text, sourceLanguage, targetLanguage);
    if (m_translationCache.contains(cacheKey)) {
        TranslationResult cachedResult = m_translationCache[cacheKey];
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        // 缓存有效期1小时（3600000毫秒）
        if (cachedResult.timestamp == 0 || (now - cachedResult.timestamp) < 3600000) {
            // 直接发射信号，请求ID用-1表示缓存命中
            emit translationFinished(-1, cachedResult);
            return -1; // 缓存命中，无实际请求ID
        } else {
            // 缓存过期，删除
            m_translationCache.remove(cacheKey);
        }
    }
    
    QStringList texts;
    texts << text;
    return translateTexts(texts, sourceLanguage, targetLanguage);
}

int TranslationCore::translateTexts(const QStringList& texts, const QString& sourceLanguage, const QString& targetLanguage)
{
    // 构建请求URL（带查询参数）
    QUrl url("https://translate.volcengineapi.com");
    QUrlQuery query;
    query.addQueryItem("Action", "TranslateText");
    query.addQueryItem("Version", "2020-06-01");
    url.setQuery(query);
    
    // 构建请求体
    QByteArray requestBody = buildTranslateRequestBody(texts, sourceLanguage, targetLanguage);
    
    // 设置请求头
    QMap<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    
    // 日志：输出翻译请求信息
    qDebug() << "[TranslationCore] Translating" << texts.size() << "texts, source:" << sourceLanguage << "target:" << targetLanguage;
    
    // 发送请求
    int requestId = m_apiClient->post(url.toString(), headers, requestBody);
    
    // 保存请求信息
    m_requestTypeMap[requestId] = "translate";
    m_requestTexts[requestId] = texts;
    m_requestSourceLang[requestId] = sourceLanguage;
    m_requestTargetLang[requestId] = targetLanguage;
    
    return requestId;
}

int TranslationCore::detectLanguage(const QString& text)
{
    // 构建请求URL（带查询参数）
    QUrl url("https://translate.volcengineapi.com");
    QUrlQuery query;
    query.addQueryItem("Action", "LangDetect");
    query.addQueryItem("Version", "2020-06-01");
    url.setQuery(query);
    
    // 构建请求体
    QByteArray requestBody = buildLanguageDetectionRequestBody(text);
    
    // 设置请求头
    QMap<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    
    // 日志：输出语言检测请求信息
    qDebug() << "[TranslationCore] Detecting language for text:" << text.left(50) << "...";
    
    // 发送请求
    int requestId = m_apiClient->post(url.toString(), headers, requestBody);
    
    // 保存请求信息
    m_requestTypeMap[requestId] = "detect";
    
    return requestId;
}

int TranslationCore::testConnection()
{
    // 日志：输出连接测试
    qDebug() << "[TranslationCore] Testing API connection...";
    // 测试连接使用简单的翻译请求
    return translateText("Hello", "", "zh");
}

void TranslationCore::fetchSupportedLanguages()
{
    qDebug() << "[TranslationCore] Fetching supported languages from API...";
    QUrl url("https://translate.volcengineapi.com");
    QUrlQuery query;
    query.addQueryItem("Action", "ListSupportedLanguages");
    query.addQueryItem("Version", "2020-06-01");
    url.setQuery(query);
    
    QMap<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    QByteArray requestBody; // 该接口可能不需要请求体
    
    int requestId = m_apiClient->post(url.toString(), headers, requestBody);
    m_requestTypeMap[requestId] = "fetchLanguages";
}

QMap<QString, QString> TranslationCore::getSupportedLanguages()
{
    if (m_supportedLanguages.isEmpty()) {
        // 若动态列表为空，返回硬编码列表并触发获取
        if (!m_languagesFetched) {
            fetchSupportedLanguages();
            m_languagesFetched = true;
        }
        // 硬编码后备列表
        QMap<QString, QString> defaultLangs;
        defaultLangs["auto"] = "自动检测";
        defaultLangs["zh"] = "中文";
        defaultLangs["en"] = "英文";
        defaultLangs["ja"] = "日语";
        defaultLangs["ko"] = "韩语";
        defaultLangs["fr"] = "法语";
        defaultLangs["de"] = "德语";
        defaultLangs["es"] = "西班牙语";
        defaultLangs["ru"] = "俄语";
        defaultLangs["pt"] = "葡萄牙语";
        defaultLangs["it"] = "意大利语";
        defaultLangs["nl"] = "荷兰语";
        defaultLangs["pl"] = "波兰语";
        defaultLangs["ar"] = "阿拉伯语";
        defaultLangs["tr"] = "土耳其语";
        defaultLangs["th"] = "泰语";
        defaultLangs["vi"] = "越南语";
        defaultLangs["id"] = "印尼语";
        defaultLangs["ms"] = "马来语";
        defaultLangs["hi"] = "印地语";
        return defaultLangs;
    }
    return m_supportedLanguages;
}

void TranslationCore::onApiRequestFinished(int requestId, bool success, const QByteArray& response, const QString& error)
{
    // 日志：输出API响应信息
    qDebug() << "[TranslationCore] API request" << requestId << "finished, success:" << success << ", error:" << error;
    
    if (!m_requestTypeMap.contains(requestId)) {
        return;
    }
    
    QString requestType = m_requestTypeMap[requestId];
    
    if (requestType == "translate") {
        // 处理翻译响应
        TranslationResult result = parseTranslateResponse(response, requestId);
        
        // 日志：输出翻译结果
        if (result.success) {
            qDebug() << "[TranslationCore] Translation succeeded, translated" << m_requestTexts.value(requestId).size() << "texts";
        } else {
            qDebug() << "[TranslationCore] Translation failed:" << result.errorMessage;
        }
        
        // 如果翻译成功，存入缓存
        if (result.success && m_requestTexts.contains(requestId)) {
            QStringList texts = m_requestTexts[requestId];
            QString sourceLang = m_requestSourceLang.value(requestId, "");
            QString targetLang = m_requestTargetLang.value(requestId, "");
            qint64 now = QDateTime::currentMSecsSinceEpoch();
            
            // 逐个缓存翻译结果
            for (int i = 0; i < texts.size() && i < result.translatedTexts.size(); ++i) {
                QString cacheKey = generateCacheKey(texts[i], sourceLang, targetLang);
                TranslationResult cachedResult = result;
                cachedResult.translatedText = result.translatedTexts[i];
                cachedResult.translatedTexts = QStringList() << result.translatedTexts[i];
                cachedResult.timestamp = now; // 设置缓存时间戳
                m_translationCache[cacheKey] = cachedResult;
            }
        }
        
        // 如果是测试连接请求，发送连接测试完成信号
        if (m_requestTexts.contains(requestId) && 
            m_requestTexts[requestId].size() == 1 && 
            m_requestTexts[requestId].first() == "Hello") {
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
            qDebug() << "[TranslationCore] Language detection succeeded:" << languageCode << "confidence:" << confidence;
            emit languageDetectionFinished(requestId, languageCode, confidence, "");
        } else {
            qDebug() << "[TranslationCore] Language detection failed:" << errorMessage;
            emit languageDetectionFinished(requestId, "", 0.0f, errorMessage);
        }
    } else if (requestType == "fetchLanguages") {
        // 处理语言列表响应
        qDebug() << "[TranslationCore] Processing supported languages response...";
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isNull() || !doc.isObject()) {
            qDebug() << "[TranslationCore] Failed to parse languages JSON";
            return;
        }
        QJsonObject root = doc.object();
        if (root.contains("Languages") && root["Languages"].isArray()) {
            QJsonArray langsArray = root["Languages"].toArray();
            m_supportedLanguages.clear();
            foreach (const QJsonValue& val, langsArray) {
                QJsonObject langObj = val.toObject();
                QString code = langObj["Code"].toString();
                QString name = langObj["Name"].toString();
                if (!code.isEmpty()) {
                    m_supportedLanguages[code] = name;
                }
            }
            m_languagesFetched = true;
            qDebug() << "[TranslationCore] Fetched" << m_supportedLanguages.size() << "languages from API";
            emit supportedLanguagesFetched(m_supportedLanguages);
        } else {
            qDebug() << "[TranslationCore] Invalid languages response format";
        }
    }
    
    // 清理请求信息
    m_requestTypeMap.remove(requestId);
    m_requestTexts.remove(requestId);
    m_requestSourceLang.remove(requestId);
    m_requestTargetLang.remove(requestId);
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
            // 火山引擎错误码可能是字符串，优先用字符串存储，转int失败则用-1
            QString errorCodeStr = error["Code"].toString();
            bool ok;
            int errorCode = errorCodeStr.toInt(&ok);
            result.errorCode = ok ? errorCode : -1;
            result.errorMessage = error["Message"].toString();
            return result;
        }
    }
    
    // 解析翻译结果
    if (rootObject.contains("TranslationList")) {
        QJsonArray translationList = rootObject["TranslationList"].toArray();
        
        if (translationList.size() > 0) {
            // 处理所有翻译结果
            for (const QJsonValue& val : translationList) {
                QJsonObject translationObj = val.toObject();
                if (translationObj.contains("Translation")) {
                    result.translatedTexts.append(translationObj["Translation"].toString());
                } else {
                    result.translatedTexts.append(QString());
                }
                
                // 取第一个结果的源语言作为整体源语言
                if (result.sourceLanguage.isEmpty() && translationObj.contains("DetectedSourceLanguage")) {
                    result.sourceLanguage = translationObj["DetectedSourceLanguage"].toString();
                }
            }
            
            // 兼容单文本场景：将第一个结果赋值给translatedText
            if (!result.translatedTexts.isEmpty()) {
                result.translatedText = result.translatedTexts.first();
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