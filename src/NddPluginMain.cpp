/**
 * @file NddPluginMain.cpp
 * @brief Notepad-- 插件入口点，实现 NDD 插件接口
 * 
 * 该文件导出 NDD_PROC_IDENTIFY 和 NDD_PROC_MAIN 函数，
 * 符合 notepad-- 的插件加载机制。
 */

#include "NddPluginInterface.h"
#include "PluginCore/TranslationEngine.h"
#include "PluginCore/ConfigManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QDialog>

// 全局插件数据
static NDD_PROC_DATA s_procData;
static QWidget* s_pMainNotepad = nullptr;
static std::function<QsciScintilla* ()> s_getCurEdit;
static std::function<bool(int, void*)> s_invokeMainFun;

// 插件核心组件
static TranslationEngine* g_translationEngine = nullptr;
static ConfigManager* g_configManager = nullptr;

// 前向声明
void initializePlugin();
void cleanupPlugin();
void onTranslateTriggered();
void onConfigureTriggered();
void showTranslationResult(const QString& original, const QString& translated, 
                          const QString& sourceLang, const QString& targetLang);
void showErrorMessage(const QString& message);

/**
 * @brief 插件标识函数
 * @param pProcData 插件数据结构指针
 * @return 是否成功
 */
NDD_EXPORT bool NDD_PROC_IDENTIFY(NDD_PROC_DATA* pProcData)
{
    if (pProcData == nullptr) {
        return false;
    }
    
    pProcData->m_strPlugName = QObject::tr("火山翻译");
    pProcData->m_strComment = QObject::tr("基于火山翻译API的文本翻译插件，支持多语言互译");
    pProcData->m_version = QString("v2.0.0");
    pProcData->m_auther = QString("Volcengine Translation Plugin");
    pProcData->m_menuType = 1;  // 1表示需要创建二级菜单
    
    return true;
}

/**
 * @brief 插件主入口函数
 * @param pNotepad 主窗口指针
 * @param strFileName 插件DLL路径
 * @param getCurEdit 获取当前编辑器的函数
 * @param pluginCallBack 回调主程序功能的函数
 * @param pProcData 插件数据（包含根菜单）
 * @return 0表示成功
 */
NDD_EXPORT int NDD_PROC_MAIN(QWidget* pNotepad, const QString& strFileName,
                            std::function<QsciScintilla* ()> getCurEdit,
                            std::function<bool(int, void*)> pluginCallBack,
                            NDD_PROC_DATA* pProcData)
{
    // 保存插件数据
    if (pProcData != nullptr) {
        s_procData = *pProcData;
    } else {
        return -1;
    }
    
    s_pMainNotepad = pNotepad;
    s_getCurEdit = getCurEdit;
    s_invokeMainFun = pluginCallBack;
    
    // 初始化插件
    initializePlugin();
    
    // 创建菜单项
    if (s_procData.m_rootMenu != nullptr) {
        QMenu* rootMenu = s_procData.m_rootMenu;
        
        // 翻译菜单项
        QAction* translateAction = new QAction(QObject::tr("翻译选中文本"), rootMenu);
        translateAction->setShortcut(QKeySequence("Ctrl+Alt+T"));
        QObject::connect(translateAction, &QAction::triggered, []() {
            onTranslateTriggered();
        });
        rootMenu->addAction(translateAction);
        
        // 分隔线
        rootMenu->addSeparator();
        
        // 配置菜单项
        QAction* configAction = new QAction(QObject::tr("配置API密钥..."), rootMenu);
        QObject::connect(configAction, &QAction::triggered, []() {
            onConfigureTriggered();
        });
        rootMenu->addAction(configAction);
        
        // 关于菜单项
        QAction* aboutAction = new QAction(QObject::tr("关于"), rootMenu);
        QObject::connect(aboutAction, &QAction::triggered, []() {
            QMessageBox::about(s_pMainNotepad, QObject::tr("关于火山翻译插件"),
                QObject::tr("火山翻译插件 v2.0.0\n\n"
                           "基于火山翻译API的文本翻译插件\n"
                           "支持多种语言互译\n\n"
                           "快捷键: Ctrl+Alt+T 翻译选中文本"));
        });
        rootMenu->addAction(aboutAction);
    }
    
    return 0;
}

/**
 * @brief 初始化插件
 */
void initializePlugin()
{
    if (g_configManager == nullptr) {
        g_configManager = new ConfigManager();
    }
    
    if (g_translationEngine == nullptr) {
        g_translationEngine = new TranslationEngine();
        
        // 连接信号
        QObject::connect(g_translationEngine, &TranslationEngine::translationFinished,
            [](int requestId, const TranslationResult& result) {
                if (result.success && !result.translatedText.isEmpty()) {
                    showTranslationResult("", result.translatedText, 
                                        result.sourceLanguage, result.targetLanguage);
                } else if (!result.success) {
                    showErrorMessage(result.errorMessage);
                }
            });
        
        QObject::connect(g_translationEngine, &TranslationEngine::connectionTestFinished,
            [](bool success, const QString& message) {
                if (success) {
                    QMessageBox::information(s_pMainNotepad, QObject::tr("测试连接"), 
                                           QObject::tr("API连接测试成功！"));
                } else {
                    QMessageBox::warning(s_pMainNotepad, QObject::tr("测试连接"), 
                                       QObject::tr("连接失败: ") + message);
                }
            });
        
        // 设置API密钥
        if (g_configManager->isConfigValid()) {
            g_translationEngine->setApiKeys(
                g_configManager->getAccessKeyId(),
                g_configManager->getAccessKeySecret()
            );
            g_translationEngine->setTimeout(g_configManager->getTimeout());
        }
    }
}

/**
 * @brief 清理插件资源
 */
void cleanupPlugin()
{
    if (g_translationEngine) {
        delete g_translationEngine;
        g_translationEngine = nullptr;
    }
    
    if (g_configManager) {
        delete g_configManager;
        g_configManager = nullptr;
    }
}

/**
 * @brief 翻译触发处理
 */
void onTranslateTriggered()
{
    if (s_getCurEdit == nullptr) {
        showErrorMessage(QObject::tr("无法获取编辑器"));
        return;
    }
    
    QsciScintilla* editor = s_getCurEdit();
    if (editor == nullptr) {
        showErrorMessage(QObject::tr("无法获取编辑器"));
        return;
    }
    
    // 获取选中文本
    QString selectedText = editor->selectedText();
    if (selectedText.isEmpty()) {
        // 如果没有选中文本，尝试获取当前行
        int line, index;
        editor->getCursorPosition(&line, &index);
        selectedText = editor->text(line);
    }
    
    if (selectedText.trimmed().isEmpty()) {
        showErrorMessage(QObject::tr("请选择要翻译的文本"));
        return;
    }
    
    // 检查配置
    if (g_configManager == nullptr || !g_configManager->isConfigValid()) {
        showErrorMessage(QObject::tr("请先配置API密钥（菜单: 火山翻译 -> 配置API密钥）"));
        return;
    }
    
    // 执行翻译
    QString sourceLang = g_configManager->getSourceLanguage();
    QString targetLang = g_configManager->getTargetLanguage();
    
    if (sourceLang == "auto" || g_configManager->getAutoDetectLanguage()) {
        sourceLang = "";  // 空字符串表示自动检测
    }
    
    qDebug() << "[NddPluginMain] Translating text:" << selectedText.left(50);
    g_translationEngine->translateText(selectedText, sourceLang, targetLang);
}

/**
 * @brief 配置触发处理
 */
void onConfigureTriggered()
{
    if (s_pMainNotepad == nullptr) {
        return;
    }
    
    // 获取当前配置
    QString currentKeyId = g_configManager ? g_configManager->getAccessKeyId() : "";
    QString currentKeySecret = g_configManager ? g_configManager->getAccessKeySecret() : "";
    
    // 显示配置对话框
    bool ok = false;
    
    // 输入 Access Key ID
    QString accessKeyId = QInputDialog::getText(s_pMainNotepad,
        QObject::tr("配置API密钥"),
        QObject::tr("请输入 Access Key ID:"),
        QLineEdit::Normal,
        currentKeyId,
        &ok);
    
    if (!ok || accessKeyId.isEmpty()) {
        return;
    }
    
    // 输入 Access Key Secret
    QString accessKeySecret = QInputDialog::getText(s_pMainNotepad,
        QObject::tr("配置API密钥"),
        QObject::tr("请输入 Access Key Secret:"),
        QLineEdit::Password,
        "",
        &ok);
    
    if (!ok || accessKeySecret.isEmpty()) {
        return;
    }
    
    // 保存配置
    if (g_configManager) {
        g_configManager->saveApiKeys(accessKeyId, accessKeySecret);
        
        // 更新翻译引擎
        if (g_translationEngine) {
            g_translationEngine->setApiKeys(accessKeyId, accessKeySecret);
        }
        
        // 测试连接
        QMessageBox::StandardButton reply = QMessageBox::question(
            s_pMainNotepad,
            QObject::tr("配置成功"),
            QObject::tr("API密钥已保存。是否测试连接？"),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes && g_translationEngine) {
            g_translationEngine->testConnection();
        }
    }
}

/**
 * @brief 显示翻译结果
 */
void showTranslationResult(const QString& original, const QString& translated,
                          const QString& sourceLang, const QString& targetLang)
{
    if (s_pMainNotepad == nullptr) {
        return;
    }
    
    // 创建结果对话框
    QDialog* dialog = new QDialog(s_pMainNotepad);
    dialog->setWindowTitle(QObject::tr("翻译结果"));
    dialog->setMinimumWidth(500);
    dialog->setMinimumHeight(300);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    // 翻译结果
    QLabel* resultLabel = new QLabel(QObject::tr("译文 (%1 → %2):")
        .arg(sourceLang.isEmpty() ? QObject::tr("自动检测") : sourceLang)
        .arg(targetLang));
    layout->addWidget(resultLabel);
    
    QTextEdit* resultText = new QTextEdit(dialog);
    resultText->setPlainText(translated);
    resultText->setReadOnly(true);
    layout->addWidget(resultText);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* copyBtn = new QPushButton(QObject::tr("复制译文"), dialog);
    QPushButton* closeBtn = new QPushButton(QObject::tr("关闭"), dialog);
    
    QObject::connect(copyBtn, &QPushButton::clicked, [translated]() {
        QApplication::clipboard()->setText(translated);
    });
    QObject::connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    
    buttonLayout->addWidget(copyBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    layout->addLayout(buttonLayout);
    
    dialog->exec();
    dialog->deleteLater();
}

/**
 * @brief 显示错误消息
 */
void showErrorMessage(const QString& message)
{
    if (s_pMainNotepad) {
        QMessageBox::warning(s_pMainNotepad, QObject::tr("火山翻译"), message);
    }
}