#ifndef TRANSLATION_ENGINE_H
#define TRANSLATION_ENGINE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QByteArray>
#include <QTimer>

#include "HttpClient.h"
#include "SignatureGenerator.h"
#include "TranslationResult.h"

/**
 * @brief 翻译引擎类
 * 负责与火山翻译API交互，处理翻译请求和响应
 */
class TranslationEngine : public QObject {
    Q_OBJECT

public:
    TranslationEngine(QObject* parent = nullptr);
    ~TranslationEngine();
    
    void setApiKeys(const QString& accessKeyId, const QString& accessKeySecret);
    void setTimeout(int timeout);
    
    int translateText(const QString& text, const QString& sourceLanguage = "", const QString& targetLanguage = "zh");
    int translateTexts(const QStringList& texts, const QString& sourceLanguage = "", const QString& targetLanguage = "zh");
    int detectLanguage(const QString& text);
    int testConnection();
    
    QMap<QString, QString> getSupportedLanguages() const;
    void fetchSupportedLanguages();

signals:
    void translationFinished(int requestId, const TranslationResult& result);
    void languageDetectionFinished(int requestId, const QString& languageCode, float confidence, const QString& error);
    void connectionTestFinished(bool success, const QString& message);
    void supportedLanguagesFetched(const QMap<QString, QString>& languages);

private slots:
    void onApiRequestFinished(int requestId, bool success, const QByteArray& response, const QString& error);

private:
    QString generateCacheKey(const QString& text, const QString& sourceLanguage, const QString& targetLanguage) {
        return QString("%1|%2|%3").arg(sourceLanguage, targetLanguage, text);
    }
    
    QByteArray buildTranslateRequestBody(const QStringList& texts, const QString& sourceLanguage, const QString& targetLanguage);
    QByteArray buildLanguageDetectionRequestBody(const QString& text);
    
    TranslationResult parseTranslateResponse(const QByteArray& response, int requestId);
    bool parseLanguageDetectionResponse(const QByteArray& response, QString& languageCode, float& confidence, QString& error);
    
    QString handleApiError(int errorCode, const QString& errorMessage);
    int sendApiRequest(const QString& action, const QByteArray& body);
    void setupApiRequest(const QString& action, const QByteArray& body);
    
    HttpClient* m_httpClient;
    SignatureGenerator* m_signatureGenerator;
    QString m_accessKeyId;
    QString m_accessKeySecret;
    int m_timeout;
    
    QMap<int, QString> m_requestTypeMap;
    QMap<int, QStringList> m_requestTexts;
    QMap<int, QString> m_requestSourceLang;
    QMap<int, QString> m_requestTargetLang;
    
    QMap<QString, TranslationResult> m_translationCache;
    QMap<QString, QString> m_supportedLanguages;
    bool m_languagesFetched;
    
    QMap<QString, QString> m_staticLanguages;
};

#endif // TRANSLATION_ENGINE_H