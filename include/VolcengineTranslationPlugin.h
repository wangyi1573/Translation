#ifndef VOLCENGINETRANSLATIONPLUGIN_H
#define VOLCENGINETRANSLATIONPLUGIN_H

#include <QObject>
#include <QString>
#include <QMainWindow>
#include <QTextEdit>
#include <QAction>
#include <QTranslator>

// 前向声明
class ConfigStorage;
class TranslationAction;
class TranslationToolbar;
class ContextMenuHandler;
class ConfigWindow;

/**
 * @brief 火山翻译插件主类
 * 实现Notepad--插件接口
 */
class VolcengineTranslationPlugin : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    VolcengineTranslationPlugin(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~VolcengineTranslationPlugin();
    
    /**
     * @brief 初始化插件
     * @param mainWindow Notepad--主窗口
     * @return 是否初始化成功
     */
    bool initialize(QMainWindow* mainWindow);
    
    /**
     * @brief 获取插件名称
     * @return 插件名称
     */
    QString name() const;
    
    /**
     * @brief 获取插件版本
     * @return 插件版本
     */
    QString version() const;
    
    /**
     * @brief 获取插件描述
     * @return 插件描述
     */
    QString description() const;

private slots:
    /**
     * @brief 翻译动作触发
     */
    void onTranslateTriggered();
    
    /**
     * @brief 配置动作触发
     */
    void onConfigureTriggered();
    
    /**
     * @brief 翻译完成处理
     * @param translatedText 翻译后的文本
     * @param sourceLanguage 源语言
     * @param targetLanguage 目标语言
     */
    void onTranslationCompleted(const QString& translatedText, const QString& sourceLanguage, const QString& targetLanguage);
    
    /**
     * @brief 翻译错误处理
     * @param errorMessage 错误信息
     */
    void onTranslationError(const QString& errorMessage);
    
    /**
     * @brief 配置已更改处理
     */
    void onConfigurationChanged();

private:
    QMainWindow* m_mainWindow;          // Notepad--主窗口
    QTextEdit* m_editor;                // 文本编辑器
    ConfigStorage* m_configStorage;     // 配置存储对象
    TranslationAction* m_translationAction; // 翻译动作对象
    TranslationToolbar* m_translationToolbar; // 翻译工具栏对象
    ContextMenuHandler* m_contextMenuHandler; // 右键菜单处理对象
    ConfigWindow* m_configWindow;       // 配置窗口
    QAction* m_configureAction;         // 配置动作
    QTranslator* m_translator;          // 翻译器
    
    /**
     * @brief 获取当前编辑器
     * @return 文本编辑器对象
     */
    QTextEdit* getCurrentEditor() const;
    
    /**
     * @brief 创建配置动作
     */
    void createConfigureAction();
    
    /**
     * @brief 初始化翻译器
     */
    void initializeTranslator();
    
    /**
     * @brief 显示翻译结果对话框
     * @param originalText 原始文本
     * @param translatedText 翻译后的文本
     * @param sourceLanguage 源语言
     * @param targetLanguage 目标语言
     */
    void showTranslationResultDialog(const QString& originalText, const QString& translatedText,
                                   const QString& sourceLanguage, const QString& targetLanguage);
    
    /**
     * @brief 显示错误消息框
     * @param message 错误消息
     */
    void showErrorMessage(const QString& message);
};

#endif // VOLCENGINETRANSLATIONPLUGIN_H