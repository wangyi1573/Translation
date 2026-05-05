#ifndef TRANSLATIONTOOLBAR_H
#define TRANSLATIONTOOLBAR_H

#include <QObject>
#include <QToolBar>
#include <QAction>
#include <QMainWindow>

/**
 * @brief 翻译工具栏类
 * 负责在Notepad--工具栏中添加翻译按钮
 */
class TranslationToolbar : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    TranslationToolbar(QObject* parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~TranslationToolbar();
    
    /**
     * @brief 初始化工具栏
     * @param mainWindow Notepad--主窗口
     */
    void initialize(QMainWindow* mainWindow);
    
    /**
     * @brief 获取翻译动作
     * @return 翻译动作对象
     */
    QAction* getTranslateAction() const;

signals:
    /**
     * @brief 翻译按钮点击信号
     */
    void translateButtonClicked();

private:
    QAction* m_translateAction; // 翻译动作
    QToolBar* m_toolBar;        // 工具栏

    /**
     * @brief 创建翻译动作
     */
    void createTranslateAction();
};

#endif // TRANSLATIONTOOLBAR_H