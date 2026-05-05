#ifndef CONTEXTMENUHANDLER_H
#define CONTEXTMENUHANDLER_H

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QTextEdit>

/**
 * @brief 右键菜单处理类
 * 负责在Notepad--的右键菜单中添加翻译选项
 */
class ContextMenuHandler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    ContextMenuHandler(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ContextMenuHandler();
    
    /**
     * @brief 初始化右键菜单
     * @param editor 文本编辑器
     */
    void initialize(QTextEdit* editor);
    
    /**
     * @brief 获取翻译动作
     * @return 翻译动作对象
     */
    QAction* getTranslateAction() const;

signals:
    /**
     * @brief 翻译菜单项点击信号
     */
    void translateMenuItemClicked();

private slots:
    /**
     * @brief 显示右键菜单前的处理
     * @param pos 菜单显示位置
     */
    void onContextMenuAboutToShow(const QPoint& pos);

private:
    QAction* m_translateAction; // 翻译动作
    QTextEdit* m_editor;        // 文本编辑器
    QMenu* m_contextMenu;       // 右键菜单

    /**
     * @brief 创建翻译动作
     */
    void createTranslateAction();
};

#endif // CONTEXTMENUHANDLER_H