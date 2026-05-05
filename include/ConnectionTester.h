#ifndef CONNECTIONTESTER_H
#define CONNECTIONTESTER_H

#include <QObject>
#include <QString>

#include "TranslationCore.h"

/**
 * @brief 连接测试器类
 * 用于测试与火山翻译API的连接是否正常
 */
class ConnectionTester : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    ConnectionTester(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ConnectionTester();
    
    /**
     * @brief 测试API连接
     * @param accessKeyId Access Key ID
     * @param accessKeySecret Access Key Secret
     * @return 测试请求ID
     */
    int testConnection(const QString& accessKeyId, const QString& accessKeySecret);
    
    /**
     * @brief 取消当前测试
     */
    void cancelTest();

signals:
    /**
     * @brief 测试完成信号
     * @param success 是否成功
     * @param message 测试结果消息
     */
    void testFinished(bool success, const QString& message);

private slots:
    /**
     * @brief 翻译核心连接测试完成处理
     * @param success 是否成功
     * @param message 测试结果消息
     */
    void onTranslationCoreTestFinished(bool success, const QString& message);

private:
    TranslationCore* m_translationCore; // 翻译核心对象
    int m_currentTestRequestId;         // 当前测试请求ID
    
    /**
     * @brief 创建翻译核心对象
     * @param accessKeyId Access Key ID
     * @param accessKeySecret Access Key Secret
     */
    void createTranslationCore(const QString& accessKeyId, const QString& accessKeySecret);
    
    /**
     * @brief 清理翻译核心对象
     */
    void cleanupTranslationCore();
};

#endif // CONNECTIONTESTER_H