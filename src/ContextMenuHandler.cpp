#include "ContextMenuHandler.h"
#include <QApplication>
#include <QStyle>

ContextMenuHandler::ContextMenuHandler(QObject* parent)
    : QObject(parent),
      m_translateAction(nullptr),
      m_editor(nullptr),
      m_contextMenu(nullptr)
{
}

ContextMenuHandler::~ContextMenuHandler()
{
}

void ContextMenuHandler::initialize(QTextEdit* editor)
{
    if (!editor) {
        return;
    }
    
    m_editor = editor;
    
    // 创建翻译动作
    createTranslateAction();
    
    // 获取编辑器的右键菜单
    m_contextMenu = m_editor->createStandardContextMenu();
    
    // 连接右键菜单显示前的信号
    connect(m_editor, &QTextEdit::customContextMenuRequested,
            this, &ContextMenuHandler::onContextMenuAboutToShow);
}

QAction* ContextMenuHandler::getTranslateAction() const
{
    return m_translateAction;
}

void ContextMenuHandler::onContextMenuAboutToShow(const QPoint& pos)
{
    if (!m_editor) {
        return;
    }
    
    // 获取编辑器的右键菜单
    QMenu* menu = m_editor->createStandardContextMenu();
    
    // 检查是否有选中文本
    QString selectedText = m_editor->textCursor().selectedText();
    bool hasSelection = !selectedText.trimmed().isEmpty();
    
    // 如果已有翻译动作，则移除
    QAction* existingAction = menu->findChild<QAction*>("volcengineTranslateAction");
    if (existingAction) {
        delete existingAction;
    }
    
    // 创建新的翻译动作
    m_translateAction = new QAction("火山翻译", menu);
    m_translateAction->setObjectName("volcengineTranslateAction");
    m_translateAction->setToolTip("翻译选中的文本");
    m_translateAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    m_translateAction->setEnabled(hasSelection);
    
    // 连接信号和槽
    connect(m_translateAction, &QAction::triggered, this, &ContextMenuHandler::translateMenuItemClicked);
    
    // 添加分隔符和翻译动作
    menu->addSeparator();
    menu->addAction(m_translateAction);
    
    // 显示菜单
    menu->exec(m_editor->mapToGlobal(pos));
    
    // 菜单关闭后删除
    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
}

void ContextMenuHandler::createTranslateAction()
{
    m_translateAction = new QAction("火山翻译", this);
    m_translateAction->setObjectName("volcengineTranslateAction");
    m_translateAction->setToolTip("翻译选中的文本");
    m_translateAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    
    // 连接信号和槽
    connect(m_translateAction, &QAction::triggered, this, &ContextMenuHandler::translateMenuItemClicked);
}