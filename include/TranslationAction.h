#ifndef TRANSLATIONACTION_H
#define TRANSLATIONACTION_H

#include <QObject>
#include <QString>

#include "TranslationCore.h"
#include "ConfigStorage.h"

/**
 * @brief 翻译动作类
 * 负责处理翻译操作的执行
 */
class TranslationAction : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param configStorage 配置存储对象
     * @param parent 父对象
     */
    TranslationAction(ConfigStorage* configStorage, QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~TranslationAction();
    
    /**
     * @brief 执行翻译
     * @param text 待翻译的文本
     * @return 翻译请求ID
     */
    int executeTranslation(const QString& text);
    
    /**
     * @brief 获取翻译核心对象
     * @return 翻译核心对象
     */
    TranslationCore* getTranslationCore() const;

signals:
    /**
     * @brief 翻译完成信号
     * @param translatedText 翻译后的文本
     * @param sourceLanguage 源语言
     * @param targetLanguage 目标语言
     */
    void translationCompleted(const QString& translatedText, const QString& sourceLanguage, const QString& targetLanguage);
    
    /**
     * @brief 翻译错误信号
     * @param errorMessage 错误信息
     */
    void translationError(const QString& errorMessage);

private slots:
    /**
     * @brief 翻译完成处理
     * @param requestId 请求ID
     * @param result 翻译结果
     */
    void onTranslationFinished(int requestId, const TranslationResult& result);

private:
    ConfigStorage* m_configStorage;    // 配置存储对象
    TranslationCore* m_translationCore; // 翻译核心对象
};

#endif // TRANSLATIONACTION_H