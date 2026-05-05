#include "ConfigWindow.h"
#include <QApplication>
#include <QStyle>
#include <QIcon>

ConfigWindow::ConfigWindow(ConfigStorage* configStorage, QWidget* parent)
    : QDialog(parent),
      m_configStorage(configStorage)
{
    // 设置窗口属性
    setWindowTitle("火山翻译配置");
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    setMinimumWidth(400);
    setModal(true);
    
    // 初始化UI
    initializeUI();
    
    // 加载当前配置
    loadCurrentConfiguration();
    
    // 创建翻译核心对象用于测试连接
    m_translationCore = new TranslationCore(m_configStorage->getAccessKeyId(),
                                           m_configStorage->getAccessKeySecret(),
                                           this);
    connect(m_translationCore, &TranslationCore::connectionTestFinished,
            this, &ConfigWindow::onConnectionTestFinished);
}

ConfigWindow::~ConfigWindow()
{
    delete m_translationCore;
}

void ConfigWindow::initializeUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建API配置组
    m_apiGroupBox = new QGroupBox("API配置");
    m_apiLayout = new QVBoxLayout();
    
    m_accessKeyIdLabel = new QLabel("Access Key ID:");
    m_accessKeyIdEdit = new QLineEdit();
    m_accessKeyIdEdit->setPlaceholderText("请输入Access Key ID");
    
    m_accessKeySecretLabel = new QLabel("Access Key Secret:");
    m_accessKeySecretEdit = new QLineEdit();
    m_accessKeySecretEdit->setEchoMode(QLineEdit::Password);
    m_accessKeySecretEdit->setPlaceholderText("请输入Access Key Secret");
    
    m_apiLayout->addWidget(m_accessKeyIdLabel);
    m_apiLayout->addWidget(m_accessKeyIdEdit);
    m_apiLayout->addWidget(m_accessKeySecretLabel);
    m_apiLayout->addWidget(m_accessKeySecretEdit);
    
    m_apiGroupBox->setLayout(m_apiLayout);
    m_mainLayout->addWidget(m_apiGroupBox);
    
    // 创建翻译配置组
    m_translationGroupBox = new QGroupBox("翻译配置");
    m_translationLayout = new QVBoxLayout();
    
    m_sourceLanguageLabel = new QLabel("源语言:");
    m_sourceLanguageComboBox = new QComboBox();
    
    m_autoDetectLanguageCheckBox = new QCheckBox("自动检测源语言");
    m_autoDetectLanguageCheckBox->setChecked(true);
    
    m_targetLanguageLabel = new QLabel("目标语言:");
    m_targetLanguageComboBox = new QComboBox();
    
    m_timeoutLabel = new QLabel("请求超时 (毫秒):");
    m_timeoutSpinBox = new QSpinBox();
    m_timeoutSpinBox->setRange(1000, 60000);
    m_timeoutSpinBox->setSingleStep(1000);
    m_timeoutSpinBox->setValue(10000);
    
    m_translationLayout->addWidget(m_sourceLanguageLabel);
    m_translationLayout->addWidget(m_sourceLanguageComboBox);
    m_translationLayout->addWidget(m_autoDetectLanguageCheckBox);
    m_translationLayout->addWidget(m_targetLanguageLabel);
    m_translationLayout->addWidget(m_targetLanguageComboBox);
    m_translationLayout->addWidget(m_timeoutLabel);
    m_translationLayout->addWidget(m_timeoutSpinBox);
    
    m_translationGroupBox->setLayout(m_translationLayout);
    m_mainLayout->addWidget(m_translationGroupBox);
    
    // 填充语言下拉框
    populateLanguageComboBoxes();
    
    // 创建按钮组
    m_buttonLayout = new QHBoxLayout();
    
    m_saveButton = new QPushButton("保存");
    m_resetButton = new QPushButton("重置");
    m_testButton = new QPushButton("测试连接");
    m_aboutButton = new QPushButton("关于");
    
    m_buttonLayout->addWidget(m_saveButton);
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addWidget(m_testButton);
    m_buttonLayout->addWidget(m_aboutButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // 连接信号和槽
    connect(m_saveButton, &QPushButton::clicked, this, &ConfigWindow::saveConfiguration);
    connect(m_resetButton, &QPushButton::clicked, this, &ConfigWindow::resetConfiguration);
    connect(m_testButton, &QPushButton::clicked, this, &ConfigWindow::testConnection);
    connect(m_aboutButton, &QPushButton::clicked, this, &ConfigWindow::showAboutDialog);
    
    // 当自动检测语言复选框状态改变时，启用/禁用源语言下拉框
    connect(m_autoDetectLanguageCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_sourceLanguageComboBox->setEnabled(!checked);
    });
}

void ConfigWindow::populateLanguageComboBoxes()
{
    // 获取支持的语言列表
    QMap<QString, QString> languages = TranslationCore::getSupportedLanguages();
    
    // 填充源语言下拉框
    m_sourceLanguageComboBox->clear();
    foreach (const QString& code, languages.keys()) {
        if (code != "auto") { // 源语言下拉框不包含"自动检测"选项
            m_sourceLanguageComboBox->addItem(languages[code], code);
        }
    }
    
    // 填充目标语言下拉框
    m_targetLanguageComboBox->clear();
    foreach (const QString& code, languages.keys()) {
        if (code != "auto") { // 目标语言下拉框不包含"自动检测"选项
            m_targetLanguageComboBox->addItem(languages[code], code);
        }
    }
    
    // 默认选择中文作为目标语言
    int zhIndex = m_targetLanguageComboBox->findData("zh");
    if (zhIndex != -1) {
        m_targetLanguageComboBox->setCurrentIndex(zhIndex);
    }
}

void ConfigWindow::loadCurrentConfiguration()
{
    // 加载API密钥
    m_accessKeyIdEdit->setText(m_configStorage->getAccessKeyId());
    m_accessKeySecretEdit->setText(m_configStorage->getAccessKeySecret());
    
    // 加载翻译配置
    QString sourceLanguage = m_configStorage->getSourceLanguage();
    if (sourceLanguage == "auto") {
        m_autoDetectLanguageCheckBox->setChecked(true);
        m_sourceLanguageComboBox->setEnabled(false);
    } else {
        m_autoDetectLanguageCheckBox->setChecked(false);
        m_sourceLanguageComboBox->setEnabled(true);
        
        int index = m_sourceLanguageComboBox->findData(sourceLanguage);
        if (index != -1) {
            m_sourceLanguageComboBox->setCurrentIndex(index);
        }
    }
    
    QString targetLanguage = m_configStorage->getTargetLanguage();
    int index = m_targetLanguageComboBox->findData(targetLanguage);
    if (index != -1) {
        m_targetLanguageComboBox->setCurrentIndex(index);
    }
    
    // 加载超时设置
    m_timeoutSpinBox->setValue(m_configStorage->getTimeout());
}

void ConfigWindow::saveConfiguration()
{
    // 保存API密钥
    QString accessKeyId = m_accessKeyIdEdit->text().trimmed();
    QString accessKeySecret = m_accessKeySecretEdit->text().trimmed();
    
    if (accessKeyId.isEmpty() || accessKeySecret.isEmpty()) {
        showMessageBox("错误", "Access Key ID和Access Key Secret不能为空", QMessageBox::Critical);
        return;
    }
    
    m_configStorage->saveApiKeys(accessKeyId, accessKeySecret);
    
    // 保存翻译配置
    QString sourceLanguage;
    if (m_autoDetectLanguageCheckBox->isChecked()) {
        sourceLanguage = "auto";
    } else {
        sourceLanguage = m_sourceLanguageComboBox->currentData().toString();
    }
    
    m_configStorage->saveSourceLanguage(sourceLanguage);
    m_configStorage->saveAutoDetectLanguage(m_autoDetectLanguageCheckBox->isChecked());
    
    QString targetLanguage = m_targetLanguageComboBox->currentData().toString();
    m_configStorage->saveTargetLanguage(targetLanguage);
    
    // 保存超时设置
    m_configStorage->saveTimeout(m_timeoutSpinBox->value());
    
    // 更新翻译核心的API密钥
    m_translationCore->setApiKeys(accessKeyId, accessKeySecret);
    
    showMessageBox("成功", "配置已保存", QMessageBox::Information);
    
    // 发送配置已更改信号
    emit configurationChanged();
}

void ConfigWindow::resetConfiguration()
{
    // 重置API密钥
    m_accessKeyIdEdit->clear();
    m_accessKeySecretEdit->clear();
    
    // 重置翻译配置
    m_autoDetectLanguageCheckBox->setChecked(true);
    m_sourceLanguageComboBox->setEnabled(false);
    
    int zhIndex = m_targetLanguageComboBox->findData("zh");
    if (zhIndex != -1) {
        m_targetLanguageComboBox->setCurrentIndex(zhIndex);
    }
    
    // 重置超时设置
    m_timeoutSpinBox->setValue(10000);
    
    showMessageBox("提示", "配置已重置为默认值", QMessageBox::Information);
}

void ConfigWindow::testConnection()
{
    QString accessKeyId = m_accessKeyIdEdit->text().trimmed();
    QString accessKeySecret = m_accessKeySecretEdit->text().trimmed();
    
    if (accessKeyId.isEmpty() || accessKeySecret.isEmpty()) {
        showMessageBox("错误", "请先输入Access Key ID和Access Key Secret", QMessageBox::Critical);
        return;
    }
    
    // 禁用测试按钮
    m_testButton->setEnabled(false);
    m_testButton->setText("测试中...");
    
    // 更新翻译核心的API密钥
    m_translationCore->setApiKeys(accessKeyId, accessKeySecret);
    
    // 测试连接
    m_translationCore->testConnection();
}

void ConfigWindow::onConnectionTestFinished(bool success, const QString& message)
{
    // 启用测试按钮
    m_testButton->setEnabled(true);
    m_testButton->setText("测试连接");
    
    if (success) {
        showMessageBox("成功", "连接成功", QMessageBox::Information);
    } else {
        showMessageBox("错误", "连接失败: " + message, QMessageBox::Critical);
    }
}

void ConfigWindow::showAboutDialog()
{
    QString aboutText = "火山翻译插件 v1.0\n\n"
                       "基于Qt 5.15.2开发\n"
                       "支持Notepad--编辑器\n\n"
                       "功能：\n"
                       "- 文本翻译\n"
                       "- 多语言支持\n"
                       "- 快捷键触发\n"
                       "- 右键菜单触发\n"
                       "- 工具栏按钮触发\n\n"
                       "© 2026 Volcengine Translation Plugin";
    
    showMessageBox("关于", aboutText, QMessageBox::Information);
}

void ConfigWindow::showMessageBox(const QString& title, const QString& message, QMessageBox::Icon icon)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}