#include "TranslationToolbar.h"
#include <QApplication>
#include <QStyle>
#include <QIcon>

TranslationToolbar::TranslationToolbar(QObject* parent)
    : QObject(parent),
      m_translateAction(nullptr),
      m_toolBar(nullptr)
{
}

TranslationToolbar::~TranslationToolbar()
{
}

void TranslationToolbar::initialize(QMainWindow* mainWindow)
{
    // 创建翻译动作
    createTranslateAction();
    
    // 查找或创建工具栏
    m_toolBar = mainWindow->findChild<QToolBar*>("pluginToolBar");
    if (!m_toolBar) {
        m_toolBar = new QToolBar("插件", mainWindow);
        mainWindow->addToolBar(m_toolBar);
    }
    
    // 添加翻译动作到工具栏
    m_toolBar->addAction(m_translateAction);
}

QAction* TranslationToolbar::getTranslateAction() const
{
    return m_translateAction;
}

void TranslationToolbar::createTranslateAction()
{
    m_translateAction = new QAction(this);
    m_translateAction->setText("火山翻译");
    m_translateAction->setToolTip("翻译选中的文本 (Ctrl+Alt+T)");
    
    // 使用标准图标
    m_translateAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    
    // 连接信号和槽
    connect(m_translateAction, &QAction::triggered, this, &TranslationToolbar::translateButtonClicked);
}