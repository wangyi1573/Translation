#ifndef TRANSLATIONRESULT_H
#define TRANSLATIONRESULT_H

#include <QString>

/**
 * @brief 翻译结果数据结构
 * 用于存储翻译操作的结果信息
 */
struct TranslationResult {
    QString translatedText;   // 翻译后的文本
    QString sourceLanguage;   // 源语言代码
    QString targetLanguage;   // 目标语言代码
    int errorCode;            // 错误代码，0表示成功
    QString errorMessage;     // 错误信息
    bool success;             // 翻译是否成功

    /**
     * @brief 构造函数
     * @param success 翻译是否成功
     * @param translatedText 翻译后的文本
     * @param sourceLanguage 源语言代码
     * @param targetLanguage 目标语言代码
     * @param errorCode 错误代码
     * @param errorMessage 错误信息
     */
    TranslationResult(bool success = false,
                     const QString& translatedText = "",
                     const QString& sourceLanguage = "",
                     const QString& targetLanguage = "",
                     int errorCode = 0,
                     const QString& errorMessage = "")
        : success(success),
          translatedText(translatedText),
          sourceLanguage(sourceLanguage),
          targetLanguage(targetLanguage),
          errorCode(errorCode),
          errorMessage(errorMessage)
    {}
};

#endif // TRANSLATIONRESULT_H