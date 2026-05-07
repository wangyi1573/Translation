#ifndef NDD_PLUGIN_INTERFACE_H
#define NDD_PLUGIN_INTERFACE_H

#include <qobject.h>
#include <qstring.h>
#include <functional>
#include <qsciscintilla.h>
#include <QAction>
#include <QMenu>

// Forward declaration - notepad-- 插件数据结构
struct NDD_PROC_DATA {
    QString m_strPlugName;      // 插件名称
    QString m_strComment;       // 插件描述
    QString m_version;          // 版本号
    QString m_auther;           // 作者
    int m_menuType;             // 菜单类型: 0=无菜单, 1=二级菜单
    QMenu* m_rootMenu;          // 根菜单指针（当 m_menuType=1 时有效）
};

#ifdef WIN32
#include <Windows.h>
#endif

// Notepad-- plugin export macros
#define NDD_EXPORTDLL

#if defined(Q_OS_WIN)
 #if defined(NDD_EXPORTDLL)
 #define NDD_EXPORT __declspec(dllexport)
 #else
 #define NDD_EXPORT __declspec(dllimport)
 #endif
#else
 #define NDD_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

NDD_EXPORT bool NDD_PROC_IDENTIFY(NDD_PROC_DATA* pProcData);
NDD_EXPORT int NDD_PROC_MAIN(QWidget* pNotepad, const QString& strFileName,
    std::function<QsciScintilla* ()> getCurEdit,
    std::function<bool(int, void*)> pluginCallBack,
    NDD_PROC_DATA* procData);

#ifdef __cplusplus
}
#endif

#endif // NDD_PLUGIN_INTERFACE_H