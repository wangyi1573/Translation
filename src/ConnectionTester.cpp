#include "ConnectionTester.h"

ConnectionTester::ConnectionTester(QObject* parent)
    : QObject(parent),
      m_translationCore(nullptr),
      m_currentTestRequestId(-1)
{
}

ConnectionTester::~ConnectionTester()
{
    cleanupTranslationCore();
}

int ConnectionTester::testConnection(const QString& accessKeyId, const QString& accessKeySecret)
{
    // 取消当前测试（如果有）
    cancelTest();
    
    // 创建翻译核心对象
    createTranslationCore(accessKeyId, accessKeySecret);
    
    // 测试连接
    m_currentTestRequestId = m_translationCore->testConnection();
    
    return m_currentTestRequestId;
}

void ConnectionTester::cancelTest()
{
    if (m_translationCore && m_currentTestRequestId != -1) {
        // 取消当前测试请求
        m_currentTestRequestId = -1;
    }
}

void ConnectionTester::onTranslationCoreTestFinished(bool success, const QString& message)
{
    // 发送测试完成信号
    emit testFinished(success, message);
    
    // 重置当前测试请求ID
    m_currentTestRequestId = -1;
}

void ConnectionTester::createTranslationCore(const QString& accessKeyId, const QString& accessKeySecret)
{
    // 清理之前的翻译核心对象
    cleanupTranslationCore();
    
    // 创建新的翻译核心对象
    m_translationCore = new TranslationCore(accessKeyId, accessKeySecret, this);
    
    // 连接信号和槽
    connect(m_translationCore, &TranslationCore::connectionTestFinished,
            this, &ConnectionTester::onTranslationCoreTestFinished);
}

void ConnectionTester::cleanupTranslationCore()
{
    if (m_translationCore) {
        delete m_translationCore;
        m_translationCore = nullptr;
    }
}