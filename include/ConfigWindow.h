#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSpinBox>

#include "ConfigStorage.h"
#include "TranslationCore.h"

/**
 * @brief 配置窗口类
 * 提供用户配置界面，用于设置火山翻译API的访问密钥和其他翻译选项
 */
class ConfigWindow : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param configStorage 配置存储对象
     * @param parent 父窗口
     */
    ConfigWindow(ConfigStorage* configStorage, QWidget* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ConfigWindow();

signals:
    /**
     * @brief 配置已更改信号
     */
    void configurationChanged();

private slots:
    /**
     * @brief 保存配置
     */
    void saveConfiguration();
    
    /**
     * @brief 重置配置
     */
    void resetConfiguration();
    
    /**
     * @brief 测试API连接
     */
    void testConnection();
    
    /**
     * @brief 连接测试完成处理
     * @param success 是否成功
     * @param message 测试结果消息
     */
    void onConnectionTestFinished(bool success, const QString& message);
    
    /**
     * @brief 显示关于对话框
     */
    void showAboutDialog();

private:
    ConfigStorage* m_configStorage;    // 配置存储对象
    TranslationCore* m_translationCore; // 翻译核心对象，用于测试连接
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    
    // API配置组
    QGroupBox* m_apiGroupBox;
    QVBoxLayout* m_apiLayout;
    QLabel* m_accessKeyIdLabel;
    QLineEdit* m_accessKeyIdEdit;
    QLabel* m_accessKeySecretLabel;
    QLineEdit* m_accessKeySecretEdit;
    
    // 翻译配置组
    QGroupBox* m_translationGroupBox;
    QVBoxLayout* m_translationLayout;
    QLabel* m_sourceLanguageLabel;
    QComboBox* m_sourceLanguageComboBox;
    QCheckBox* m_autoDetectLanguageCheckBox;
    QLabel* m_targetLanguageLabel;
    QComboBox* m_targetLanguageComboBox;
    QLabel* m_timeoutLabel;
    QSpinBox* m_timeoutSpinBox;
    
    // 按钮组
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_saveButton;
    QPushButton* m_resetButton;
    QPushButton* m_testButton;
    QPushButton* m_aboutButton;
    
    /**
     * @brief 初始化UI
     */
    void initializeUI();
    
    /**
     * @brief 填充语言下拉框
     */
    void populateLanguageComboBoxes();
    
    /**
     * @brief 加载当前配置
     */
    void loadCurrentConfiguration();
    
    /**
     * @brief 显示消息框
     * @param title 标题
     * @param message 消息
     * @param icon 图标类型
     */
    void showMessageBox(const QString& title, const QString& message, QMessageBox::Icon icon);
};

#endif // CONFIGWINDOW_H