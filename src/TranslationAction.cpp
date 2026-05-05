#include "TranslationAction.h"

TranslationAction::TranslationAction(ConfigStorage* configStorage, QObject* parent)
    : QObject(parent),
      m_configStorage(configStorage)
{
    // 创建翻译核心对象
    m_translationCore = new TranslationCore(m_configStorage->getAccessKeyId(),
                                           m_configStorage->getAccessKeySecret(),
                                           this);
    
    // 设置超时时间
    m_translationCore->setTimeout(m_configStorage->getTimeout());
    
    // 连接信号和槽
    connect(m_translationCore, &TranslationCore::translationFinished,
            this, &TranslationAction::onTranslationFinished);
}

TranslationAction::~TranslationAction()
{
    delete m_translationCore;
}

int TranslationAction::executeTranslation(const QString& text)
{
    if (text.trimmed().isEmpty()) {
        emit translationError("请选择要翻译的文本");
        return -1;
    }
    
    // 检查API密钥是否已配置
    QString accessKeyId = m_configStorage->getAccessKeyId();
    QString accessKeySecret = m_configStorage->getAccessKeySecret();
    
    if (accessKeyId.isEmpty() || accessKeySecret.isEmpty()) {
        emit translationError("请先配置API密钥");
        return -1;
    }
    
    // 获取源语言和目标语言设置
    QString sourceLanguage = m_configStorage->getSourceLanguage();
    QString targetLanguage = m_configStorage->getTargetLanguage();
    
    // 如果源语言设置为自动检测，则传递空字符串
    if (sourceLanguage == "auto" || m_configStorage->getAutoDetectLanguage()) {
        sourceLanguage = "";
    }
    
    // 执行翻译
    return m_translationCore->translateText(text, sourceLanguage, targetLanguage);
}

TranslationCore* TranslationAction::getTranslationCore() const
{
    return m_translationCore;
}

void TranslationAction::onTranslationFinished(int requestId, const TranslationResult& result)
{
    if (result.success) {
        // 翻译成功
        emit translationCompleted(result.translatedText, result.sourceLanguage, result.targetLanguage);
    } else {
        // 翻译失败
        emit translationError(result.errorMessage);
    }
}