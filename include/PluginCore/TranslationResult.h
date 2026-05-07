#ifndef TRANSLATION_RESULT_H
#define TRANSLATION_RESULT_H

#include <QString>
#include <QStringList>

/**
 * @brief 翻译结果数据结构
 * 用于存储翻译操作的结果信息
 */
struct TranslationResult {
    QString translatedText;       // 翻译后的文本（单文本兼容字段）
    QStringList translatedTexts;  // 翻译后的文本列表（多文本结果）
    QString sourceLanguage;       // 源语言代码
    QString targetLanguage;       // 目标语言代码
    int errorCode;                // 错误代码，0表示成功
    QString errorMessage;         // 错误信息
    bool success;                 // 翻译是否成功
    qint64 timestamp;             // 缓存时间戳（毫秒），0表示无缓存

    TranslationResult(bool success = false,
                     const QString& translatedText = "",
                     const QString& sourceLanguage = "",
                     const QString& targetLanguage = "",
                     int errorCode = 0,
                     const QString& errorMessage = "")
        : translatedText(translatedText),
          sourceLanguage(sourceLanguage),
          targetLanguage(targetLanguage),
          errorCode(errorCode),
          errorMessage(errorMessage),
          success(success),
          timestamp(0)
    {}
};

#endif // TRANSLATION_RESULT_H