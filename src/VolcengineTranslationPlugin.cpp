#include "VolcengineTranslationPlugin.h"
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QDialog>
#include <QClipboard>
#include <QKeySequence>
#include <QAction>
#include <QMenu>
#include <QMenuBar>

// 包含其他组件头文件
#include "ConfigStorage.h"
#include "TranslationAction.h"
#include "TranslationToolbar.h"
#include "ContextMenuHandler.h"
#include "ConfigWindow.h"

VolcengineTranslationPlugin::VolcengineTranslationPlugin(QObject* parent)
    : QObject(parent),
      m_mainWindow(nullptr),
      m_editor(nullptr),
      m_configStorage(nullptr),
      m_translationAction(nullptr),
      m_translationToolbar(nullptr),
      m_contextMenuHandler(nullptr),
      m_configWindow(nullptr),
      m_configureAction(nullptr),
      m_translator(nullptr)
{
}

VolcengineTranslationPlugin::~VolcengineTranslationPlugin()
{
    // 清理资源
    delete m_configStorage;
    delete m_translationAction;
    delete m_translationToolbar;
    delete m_contextMenuHandler;
    delete m_configWindow;
    delete m_configureAction;
    delete m_translator;
}

bool VolcengineTranslationPlugin::initialize(QMainWindow* mainWindow)
{
    if (!mainWindow) {
        return false;
    }
    
    m_mainWindow = mainWindow;
    
    // 初始化翻译器
    initializeTranslator();
    
    // 创建配置存储对象
    m_configStorage = new ConfigStorage();
    
    // 创建翻译动作对象
    m_translationAction = new TranslationAction(m_configStorage, this);
    
    // 连接翻译信号和槽
    connect(m_translationAction, &TranslationAction::translationCompleted,
            this, &VolcengineTranslationPlugin::onTranslationCompleted);
    connect(m_translationAction, &TranslationAction::translationError,
            this, &VolcengineTranslationPlugin::onTranslationError);
    
    // 创建并初始化工具栏
    m_translationToolbar = new TranslationToolbar(this);
    m_translationToolbar->initialize(m_mainWindow);
    
    // 连接工具栏信号
    connect(m_translationToolbar, &TranslationToolbar::translateButtonClicked,
            this, &VolcengineTranslationPlugin::onTranslateTriggered);
    
    // 获取当前编辑器
    m_editor = getCurrentEditor();
    if (m_editor) {
        // 创建并初始化右键菜单处理器
        m_contextMenuHandler = new ContextMenuHandler(this);
        m_contextMenuHandler->initialize(m_editor);
        
        // 连接右键菜单信号
        connect(m_contextMenuHandler, &ContextMenuHandler::translateMenuItemClicked,
                this, &VolcengineTranslationPlugin::onTranslateTriggered);
    }
    
    // 创建配置动作
    createConfigureAction();
    
    // 创建快捷键
    QAction* translateShortcutAction = new QAction(this);
    translateShortcutAction->setShortcut(QKeySequence("Ctrl+Alt+T"));
    translateShortcutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_mainWindow->addAction(translateShortcutAction);
    
    // 连接快捷键信号
    connect(translateShortcutAction, &QAction::triggered,
            this, &VolcengineTranslationPlugin::onTranslateTriggered);
    
    return true;
}

QString VolcengineTranslationPlugin::name() const
{
    return tr("火山翻译");
}

QString VolcengineTranslationPlugin::version() const
{
    return "1.0.0";
}

QString VolcengineTranslationPlugin::description() const
{
    return tr("基于火山翻译API的文本翻译插件，支持多种触发方式。");
}

void VolcengineTranslationPlugin::onTranslateTriggered()
{
    // 获取当前编辑器
    QTextEdit* editor = getCurrentEditor();
    if (!editor) {
        showErrorMessage(tr("无法获取编辑器"));
        return;
    }
    
    // 获取选中文本
    QString selectedText = editor->textCursor().selectedText();
    if (selectedText.trimmed().isEmpty()) {
        showErrorMessage(tr("请选择要翻译的文本"));
        return;
    }
    
    // 执行翻译
    m_translationAction->executeTranslation(selectedText);
}

void VolcengineTranslationPlugin::onConfigureTriggered()
{
    // 创建并显示配置窗口
    if (!m_configWindow) {
        m_configWindow = new ConfigWindow(m_configStorage, m_mainWindow);
        
        // 连接配置更改信号
        connect(m_configWindow, &ConfigWindow::configurationChanged,
                this, &VolcengineTranslationPlugin::onConfigurationChanged);
    }
    
    m_configWindow->show();
    m_configWindow->raise();
    m_configWindow->activateWindow();
}

void VolcengineTranslationPlugin::onTranslationCompleted(const QString& translatedText, const QString& sourceLanguage, const QString& targetLanguage)
{
    // 获取当前编辑器和选中文本
    QTextEdit* editor = getCurrentEditor();
    if (!editor) {
        return;
    }
    
    QString originalText = editor->textCursor().selectedText();
    
    // 显示翻译结果对话框
    showTranslationResultDialog(originalText, translatedText, sourceLanguage, targetLanguage);
}

void VolcengineTranslationPlugin::onTranslationError(const QString& errorMessage)
{
    showErrorMessage(errorMessage);
}

void VolcengineTranslationPlugin::onConfigurationChanged()
{
    // 配置已更改，可以在这里更新插件状态
    // 例如，重新初始化翻译核心等
}

QTextEdit* VolcengineTranslationPlugin::getCurrentEditor() const
{
    // 尝试获取当前编辑器
    // 这里假设Notepad--的编辑器类名为ScintillaEditView或QTextEdit
    QTextEdit* editor = m_mainWindow->findChild<QTextEdit*>("editor");
    if (!editor) {
        // 尝试其他可能的编辑器类名
        editor = m_mainWindow->findChild<QTextEdit*>();
    }
    
    return editor;
}

void VolcengineTranslationPlugin::createConfigureAction()
{
    // 创建配置动作
    m_configureAction = new QAction(tr("火山翻译配置"), this);
    m_configureAction->setToolTip(tr("配置火山翻译插件"));
    m_configureAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    
    // 连接配置动作信号
    connect(m_configureAction, &QAction::triggered,
            this, &VolcengineTranslationPlugin::onConfigureTriggered);
    
    // 将配置动作添加到插件菜单
    QMenu* pluginMenu = nullptr;
    
    // 尝试查找插件菜单
    QMenuBar* menuBar = m_mainWindow->menuBar();
    if (menuBar) {
        // 查找"插件"菜单
        foreach (QAction* action, menuBar->actions()) {
            if (action->text().contains(tr("插件")) || action->text().contains("Plugin")) {
                pluginMenu = action->menu();
                break;
            }
        }
        
        // 如果没有找到插件菜单，创建一个
        if (!pluginMenu) {
            pluginMenu = new QMenu(tr("插件"), m_mainWindow);
            menuBar->addMenu(pluginMenu);
        }
        
        // 添加配置动作到插件菜单
        pluginMenu->addAction(m_configureAction);
    }
}

void VolcengineTranslationPlugin::initializeTranslator()
{
    // 创建翻译器
    m_translator = new QTranslator(this);
    
    // 加载翻译文件（如果有）
    // 这里可以根据系统语言加载相应的翻译文件
    QString language = QLocale::system().name();
    QString translationFile = QString("volcengine_translation_%1.qm").arg(language);
    
    // 尝试加载翻译文件
    // if (m_translator->load(translationFile, ":/translations/")) {
    //     QApplication::installTranslator(m_translator);
    // }
}

void VolcengineTranslationPlugin::showTranslationResultDialog(const QString& originalText, const QString& translatedText,
                                                           const QString& sourceLanguage, const QString& targetLanguage)
{
    // 创建对话框
    QDialog* dialog = new QDialog(m_mainWindow);
    dialog->setWindowTitle(tr("翻译结果"));
    dialog->setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    dialog->setMinimumWidth(600);
    dialog->setMinimumHeight(400);
    
    // 创建布局
    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
    
    // 源文本部分
    QGroupBox* sourceGroupBox = new QGroupBox(tr("原文 (%1)").arg(sourceLanguage));
    QVBoxLayout* sourceLayout = new QVBoxLayout(sourceGroupBox);
    QTextEdit* sourceTextEdit = new QTextEdit(sourceGroupBox);
    sourceTextEdit->setPlainText(originalText);
    sourceTextEdit->setReadOnly(true);
    sourceTextEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    sourceLayout->addWidget(sourceTextEdit);
    mainLayout->addWidget(sourceGroupBox);
    
    // 翻译文本部分
    QGroupBox* translatedGroupBox = new QGroupBox(tr("译文 (%1)").arg(targetLanguage));
    QVBoxLayout* translatedLayout = new QVBoxLayout(translatedGroupBox);
    QTextEdit* translatedTextEdit = new QTextEdit(translatedGroupBox);
    translatedTextEdit->setPlainText(translatedText);
    translatedTextEdit->setReadOnly(true);
    translatedTextEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    translatedLayout->addWidget(translatedTextEdit);
    mainLayout->addWidget(translatedGroupBox);
    
    // 按钮部分
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* copyButton = new QPushButton(tr("复制译文"), dialog);
    QPushButton* closeButton = new QPushButton(tr("关闭"), dialog);
    
    buttonLayout->addWidget(copyButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号和槽
    connect(copyButton, &QPushButton::clicked, [translatedText]() {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(translatedText);
    });
    
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);
    
    // 显示对话框
    dialog->exec();
    
    // 对话框关闭后删除
    delete dialog;
}

void VolcengineTranslationPlugin::showErrorMessage(const QString& message)
{
    QMessageBox::critical(m_mainWindow, tr("错误"), message);
}