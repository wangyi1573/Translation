#pragma once
#include <QWidget>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <Qsci/qsciscintilla.h>
#include "ui_volcenginetranslate.h"

class VolcengineApi;
class VolcengineTransConfig;

class VolcengineTranslate : public QWidget {
    Q_OBJECT

public:
    explicit VolcengineTranslate(QWidget* parent, QWidget* pNotepad);
    ~VolcengineTranslate();

private slots:
    void onConfig();
    void onTranslateClicked();
    void onSwapLang();
    void onApiTranslateFinished(bool success, const QString& translated,
                                const QString& sourceLang, const QString& error);

private:
    void loadConfig();
    QString getSelectedText();

    Ui_VolcengineTranslate ui;
    QPointer<VolcengineApi> m_api;
    QPointer<VolcengineTransConfig> m_configDialog;
    QWidget* m_pNotepad;
    QString m_accessKeyId;
    QString m_accessKeySecret;
};

// 外部获取编辑器接口
extern std::function<QsciScintilla* (QWidget*)> s_getCurEdit;