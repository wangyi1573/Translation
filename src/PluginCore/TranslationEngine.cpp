#include "../include/PluginCore/TranslationEngine.h"
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDateTime>

TranslationEngine::TranslationEngine(QObject* parent)
    : QObject(parent),
      m_httpClient(new HttpClient(this)),
      m_signatureGenerator(nullptr),
      m_timeout(30000),
      m_languagesFetched(false)
{
    connect(m_httpClient, &HttpClient::requestFinished, this, &TranslationEngine::onApiRequestFinished);
    
    // 初始化静态语言列表
    m_staticLanguages["auto"] = "自动检测";
    m_staticLanguages["zh"] = "中文";
    m_staticLanguages["en"] = "英文";
    m_staticLanguages["ja"] = "日语";
    m_staticLanguages["ko"] = "韩语";
    m_staticLanguages["fr"] = "法语";
    m_staticLanguages["de"] = "德语";
    m_staticLanguages["es"] = "西班牙语";
    m_staticLanguages["ru"] = "俄语";
    m_staticLanguages["pt"] = "葡萄牙语";
    m_staticLanguages["it"] = "意大利语";
}

TranslationEngine::~TranslationEngine()
{
    delete m_httpClient;
    delete m_signatureGenerator;
}

void TranslationEngine::setApiKeys(const QString& accessKeyId, const QString& accessKeySecret)
{
    m_accessKeyId = accessKeyId;
    m_accessKeySecret = accessKeySecret;
    
    delete m_signatureGenerator;
    m_signatureGenerator = new SignatureGenerator(accessKeyId, accessKeySecret);
}

void TranslationEngine::setTimeout(int timeout)
{
    m_timeout = timeout;
    m_httpClient->setTimeout(timeout);
}

int TranslationEngine::translateText(const QString& text, const QString& sourceLanguage, const QString& targetLanguage)
{
    // 检查缓存
    QString cacheKey = generateCacheKey(text, sourceLanguage, targetLanguage);
    if (m_translationCache.contains(cacheKey)) {
        TranslationResult cachedResult = m_translationCache[cacheKey];
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (cachedResult.timestamp == 0 || (now - cachedResult.timestamp) < 3600000) {
            emit translationFinished(-1, cachedResult);
            return -1;
        } else {
            m_translationCache.remove(cacheKey);
        }
    }
    
    QStringList texts;
    texts << text;
    return translateTexts(texts, sourceLanguage, targetLanguage);
}

int TranslationEngine::translateTexts(const QStringList& texts, const QString& sourceLanguage, const QString& targetLanguage)
{
    if (!m_signatureGenerator) {
        emit connectionTestFinished(false, "请先配置API密钥");
        return -1;
    }
    
    // 构建请求URL
    QString url = "https://translate.volcengineapi.com/?Action=TranslateText&Version=2020-06-01";
    
    // 构建请求体
    QByteArray requestBody = buildTranslateRequestBody(texts, sourceLanguage, targetLanguage);
    
    // 计算请求体哈希
    QString contentSha256 = m_signatureGenerator->sha256(requestBody);
    
    // 设置请求头
    QMap<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    headers["Host"] = "translate.volcengineapi.com";
    headers["X-Date"] = SignatureGenerator::getCurrentTimeISO8601();
    headers["X-Content-Sha256"] = contentSha256;
    
    // 生成签名
    QMap<QString, QString> queryParams;
    queryParams["Action"] = "TranslateText";
    queryParams["Version"] = "2020-06-01";
    
    QString authorization = m_signatureGenerator->generateAuthorizationHeader(
        "POST", "/", queryParams, headers, requestBody);
    headers["Authorization"] = authorization;
    
    qDebug() << "[TranslationEngine] Translating" << texts.size() << "texts";
    
    int requestId = m_httpClient->post(url, headers, requestBody);
    
    m_requestTypeMap[requestId] = "translate";
    m_requestTexts[requestId] = texts;
    m_requestSourceLang[requestId] = sourceLanguage;
    m_requestTargetLang[requestId] = targetLanguage;
    
    return requestId;
}

int TranslationEngine::detectLanguage(const QString& text)
{
    if (!m_signatureGenerator) {
        return -1;
    }
    
    QString url = "https://translate.volcengineapi.com/?Action=LangDetect&Version=2020-06-01";
    QByteArray requestBody = buildLanguageDetectionRequestBody(text);
    
    QString contentSha256 = m_signatureGenerator->sha256(requestBody);
    
    QMap<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    headers["Host"] = "translate.volcengineapi.com";
    headers["X-Date"] = SignatureGenerator::getCurrentTimeISO8601();
    headers["X-Content-Sha256"] = contentSha256;
    
    QMap<QString, QString> queryParams;
    queryParams["Action"] = "LangDetect";
    queryParams["Version"] = "2020-06-01";
    
    QString authorization = m_signatureGenerator->generateAuthorizationHeader(
        "POST", "/", queryParams, headers, requestBody);
    headers["Authorization"] = authorization;
    
    qDebug() << "[TranslationEngine] Detecting language";
    
    int requestId = m_httpClient->post(url, headers, requestBody);
    m_requestTypeMap[requestId] = "detect";
    
    return requestId;
}

int TranslationEngine::testConnection()
{
    qDebug() << "[TranslationEngine] Testing connection...";
    return translateText("Hello", "", "zh");
}

QMap<QString, QString> TranslationEngine::getSupportedLanguages() const
{
    if (!m_supportedLanguages.isEmpty()) {
        return m_supportedLanguages;
    }
    return m_staticLanguages;
}

void TranslationEngine::fetchSupportedLanguages()
{
    if (!m_signatureGenerator) {
        return;
    }
    
    qDebug() << "[TranslationEngine] Fetching supported languages...";
    
    QString url = "https://translate.volcengineapi.com/?Action=ListSupportedLanguages&Version=2020-06-01";
    
    QMap<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    headers["Host"] = "translate.volcengineapi.com";
    headers["X-Date"] = SignatureGenerator::getCurrentTimeISO8601();
    headers["X-Content-Sha256"] = m_signatureGenerator->sha256("");
    
    QMap<QString, QString> queryParams;
    queryParams["Action"] = "ListSupportedLanguages";
    queryParams["Version"] = "2020-06-01";
    
    QString authorization = m_signatureGenerator->generateAuthorizationHeader(
        "POST", "/", queryParams, headers, QByteArray());
    headers["Authorization"] = authorization;
    
    int requestId = m_httpClient->post(url, headers, QByteArray());
    m_requestTypeMap[requestId] = "fetchLanguages";
}

void TranslationEngine::onApiRequestFinished(int requestId, bool success, const QByteArray& response, const QString& error)
{
    qDebug() << "[TranslationEngine] Request" << requestId << "finished, success:" << success;
    
    if (!m_requestTypeMap.contains(requestId)) {
        return;
    }
    
    QString requestType = m_requestTypeMap[requestId];
    
    if (requestType == "translate") {
        TranslationResult result;
        
        if (!success) {
            result.success = false;
            result.errorMessage = error;
        } else {
            result = parseTranslateResponse(response, requestId);
        }
        
        // 缓存成功的翻译结果
        if (result.success && m_requestTexts.contains(requestId)) {
            QStringList texts = m_requestTexts[requestId];
            QString sourceLang = m_requestSourceLang.value(requestId, "");
            QString targetLang = m_requestTargetLang.value(requestId, "");
            qint64 now = QDateTime::currentMSecsSinceEpoch();
            
            for (int i = 0; i < texts.size() && i < result.translatedTexts.size(); ++i) {
                QString cacheKey = generateCacheKey(texts[i], sourceLang, targetLang);
                TranslationResult cachedResult = result;
                cachedResult.translatedText = result.translatedTexts[i];
                cachedResult.translatedTexts = QStringList() << result.translatedTexts[i];
                cachedResult.timestamp = now;
                m_translationCache[cacheKey] = cachedResult;
            }
        }
        
        // 检查是否为测试连接请求
        if (m_requestTexts.contains(requestId) && 
            m_requestTexts[requestId].size() == 1 && 
            m_requestTexts[requestId].first() == "Hello") {
            emit connectionTestFinished(result.success, result.success ? "连接成功" : result.errorMessage);
        } else {
            emit translationFinished(requestId, result);
        }
    } else if (requestType == "detect") {
        QString languageCode;
        float confidence = 0.0f;
        QString errorMessage;
        
        if (success) {
            bool parseSuccess = parseLanguageDetectionResponse(response, languageCode, confidence, errorMessage);
            if (parseSuccess) {
                qDebug() << "[TranslationEngine] Language detected:" << languageCode;
                emit languageDetectionFinished(requestId, languageCode, confidence, "");
            } else {
                emit languageDetectionFinished(requestId, "", 0.0f, errorMessage);
            }
        } else {
            emit languageDetectionFinished(requestId, "", 0.0f, error);
        }
    } else if (requestType == "fetchLanguages") {
        if (success) {
            QJsonDocument doc = QJsonDocument::fromJson(response);
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject root = doc.object();
                if (root.contains("Languages") && root["Languages"].isArray()) {
                    QJsonArray langsArray = root["Languages"].toArray();
                    m_supportedLanguages.clear();
                    for (const QJsonValue& val : langsArray) {
                        QJsonObject langObj = val.toObject();
                        QString code = langObj["Code"].toString();
                        QString name = langObj["Name"].toString();
                        if (!code.isEmpty()) {
                            m_supportedLanguages[code] = name;
                        }
                    }
                    m_languagesFetched = true;
                    qDebug() << "[TranslationEngine] Fetched" << m_supportedLanguages.size() << "languages";
                    emit supportedLanguagesFetched(m_supportedLanguages);
                }
            }
        }
    }
    
    // 清理请求信息
    m_requestTypeMap.remove(requestId);
    m_requestTexts.remove(requestId);
    m_requestSourceLang.remove(requestId);
    m_requestTargetLang.remove(requestId);
}

QByteArray TranslationEngine::buildTranslateRequestBody(const QStringList& texts, const QString& sourceLanguage, const QString& targetLanguage)
{
    QJsonObject requestBody;
    requestBody["TargetLanguage"] = targetLanguage;
    
    if (!sourceLanguage.isEmpty() && sourceLanguage != "auto") {
        requestBody["SourceLanguage"] = sourceLanguage;
    }
    
    QJsonArray textArray;
    for (const QString& text : texts) {
        textArray.append(text);
    }
    requestBody["TextList"] = textArray;
    
    QJsonDocument doc(requestBody);
    return doc.toJson(QJsonDocument::Compact);
}

QByteArray TranslationEngine::buildLanguageDetectionRequestBody(const QString& text)
{
    QJsonObject requestBody;
    requestBody["Text"] = text;
    
    QJsonDocument doc(requestBody);
    return doc.toJson(QJsonDocument::Compact);
}

TranslationResult TranslationEngine::parseTranslateResponse(const QByteArray& response, int requestId)
{
    TranslationResult result;
    
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (doc.isNull()) {
        result.success = false;
        result.errorCode = -1;
        result.errorMessage = "无效的JSON响应";
        return result;
    }
    
    QJsonObject rootObject = doc.object();
    
    // 检查错误
    if (rootObject.contains("ResponseMetadata")) {
        QJsonObject responseMetadata = rootObject["ResponseMetadata"].toObject();
        if (responseMetadata.contains("Error")) {
            QJsonObject error = responseMetadata["Error"].toObject();
            result.success = false;
            QString errorCodeStr = error["Code"].toString();
            bool ok;
            result.errorCode = errorCodeStr.toInt(&ok) ? errorCodeStr.toInt() : -1;
            result.errorMessage = error["Message"].toString();
            return result;
        }
    }
    
    // 解析翻译结果
    if (rootObject.contains("TranslationList")) {
        QJsonArray translationList = rootObject["TranslationList"].toArray();
        
        if (translationList.size() > 0) {
            for (const QJsonValue& val : translationList) {
                QJsonObject translationObj = val.toObject();
                if (translationObj.contains("Translation")) {
                    result.translatedTexts.append(translationObj["Translation"].toString());
                } else {
                    result.translatedTexts.append(QString());
                }
                
                if (result.sourceLanguage.isEmpty() && translationObj.contains("DetectedSourceLanguage")) {
                    result.sourceLanguage = translationObj["DetectedSourceLanguage"].toString();
                }
            }
            
            if (!result.translatedTexts.isEmpty()) {
                result.translatedText = result.translatedTexts.first();
            }
            
            if (rootObject.contains("TargetLanguage")) {
                result.targetLanguage = rootObject["TargetLanguage"].toString();
            }
            
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

bool TranslationEngine::parseLanguageDetectionResponse(const QByteArray& response, QString& languageCode, float& confidence, QString& error)
{
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (doc.isNull()) {
        error = "无效的JSON响应";
        return false;
    }
    
    QJsonObject rootObject = doc.object();
    
    if (rootObject.contains("ResponseMetadata")) {
        QJsonObject responseMetadata = rootObject["ResponseMetadata"].toObject();
        if (responseMetadata.contains("Error")) {
            QJsonObject errorObj = responseMetadata["Error"].toObject();
            error = errorObj["Message"].toString();
            return false;
        }
    }
    
    if (rootObject.contains("DetectedLanguage")) {
        languageCode = rootObject["DetectedLanguage"].toString();
        confidence = rootObject.contains("Confidence") ? rootObject["Confidence"].toDouble() : 1.0f;
        return true;
    }
    
    error = "响应中没有语言检测结果";
    return false;
}

QString TranslationEngine::handleApiError(int errorCode, const QString& errorMessage)
{
    switch (errorCode) {
    case -400:
        return "请求参数错误: " + errorMessage;
    case -415:
        return "不支持的翻译: " + errorMessage;
    case -429:
        return "请求过于频繁，请稍后再试";
    case -500:
        return "翻译服务内部错误: " + errorMessage;
    default:
        return "翻译失败 (错误码: " + QString::number(errorCode) + "): " + errorMessage;
    }
}