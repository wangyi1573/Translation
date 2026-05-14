#include "volcenginetranslate.h"
#include "volcengineapi.h"
#include "volcenginetransconfig.h"
#include <QSettings>
#include <QMessageBox>
#include <QClipboard>

static const char* CONFIG_KEY_ID = "VolcengineTranslate/AccessKeyId";
static const char* CONFIG_KEY_SECRET = "VolcengineTranslate/AccessKeySecret";

VolcengineTranslate::VolcengineTranslate(QWidget* parent, QWidget* pNotepad)
    : QWidget(parent)
    , m_pNotepad(pNotepad)
{
    ui.setupUi(this);

    loadConfig();

    if (m_api) {
        delete m_api;
    }
    m_api = new VolcengineApi(this);
    m_api->setCredentials(m_accessKeyId, m_accessKeySecret);
    connect(m_api, &VolcengineApi::translateFinished,
            this, &VolcengineTranslate::onApiTranslateFinished);

    connect(ui.btnConfig, &QPushButton::clicked, this, &VolcengineTranslate::onConfig);
    connect(ui.btnTranslateSel, &QPushButton::clicked, this, &VolcengineTranslate::onTranslateClicked);
    connect(ui.btnSwap, &QPushButton::clicked, this, &VolcengineTranslate::onSwapLang);
    connect(ui.btnCopy, &QPushButton::clicked, this, [this]() {
        QString text = ui.textEditTarget->toPlainText();
        if (!text.isEmpty()) {
            QApplication::clipboard()->setText(text);
            ui.statusBar->showMessage(QString::fromLocal8Bit("已复制到剪贴板"), 3000);
        }
    });
    connect(ui.btnReplace, &QPushButton::clicked, this, [this]() {
        QString translated = ui.textEditTarget->toPlainText();
        if (!translated.isEmpty() && s_getCurEdit && m_pNotepad) {
            QsciScintilla* edit = s_getCurEdit(m_pNotepad);
            if (edit) {
                QString selText = edit->selectedText();
                if (!selText.isEmpty()) {
                    edit->replaceSelectedText(translated);
                }
            }
        }
    });

    // Combo language codes
    ui.comboSourceLang->setCurrentIndex(0);
    ui.comboTargetLang->setCurrentIndex(0);
}

VolcengineTranslate::~VolcengineTranslate() {}

void VolcengineTranslate::loadConfig() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Notepad--", "config");
    m_accessKeyId = settings.value(CONFIG_KEY_ID, "").toString();
    m_accessKeySecret = settings.value(CONFIG_KEY_SECRET, "").toString();
}

QString VolcengineTranslate::getSelectedText() {
    if (s_getCurEdit && m_pNotepad) {
        QsciScintilla* edit = s_getCurEdit(m_pNotepad);
        if (edit) {
            return edit->selectedText();
        }
    }
    return "";
}

void VolcengineTranslate::onConfig() {
    if (!m_configDialog) {
        m_configDialog = new VolcengineTransConfig(this);
    }
    m_configDialog->loadConfig();
    m_configDialog->show();
    m_configDialog->raise();
    m_configDialog->activateWindow();
}

void VolcengineTranslate::onTranslateClicked() {
    QString text = getSelectedText();
    if (text.isEmpty()) {
        text = ui.textEditSource->toPlainText().trimmed();
    }

    if (text.isEmpty()) {
        QMessageBox::warning(this, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("请选择要翻译的文本或输入文本"));
        return;
    }

    ui.textEditSource->setPlainText(text);

    // Get language codes
    static const QMap<int, QString> langMap = {
        {0, "auto"}, {1, "zh"}, {2, "en"}, {3, "ja"},
        {4, "ko"}, {5, "fr"}, {6, "de"}, {7, "ru"}, {8, "es"}
    };

    QString sourceLang = langMap.value(ui.comboSourceLang->currentIndex(), "auto");
    QString targetLang = langMap.value(ui.comboTargetLang->currentIndex(), "zh");

    ui.statusBar->showMessage(QString::fromLocal8Bit("正在翻译..."));
    m_api->translate(text, sourceLang, targetLang);
}

void VolcengineTranslate::onSwapLang() {
    int srcIdx = ui.comboSourceLang->currentIndex();
    int tgtIdx = ui.comboTargetLang->currentIndex();
    ui.comboSourceLang->setCurrentIndex(tgtIdx);
    ui.comboTargetLang->setCurrentIndex(srcIdx);
}

void VolcengineTranslate::onApiTranslateFinished(bool success, const QString& translated,
                                                   const QString& sourceLang, const QString& error) {
    if (success) {
        ui.textEditTarget->setPlainText(translated);
        ui.statusBar->showMessage(QString::fromLocal8Bit("翻译完成 (%1→%2)")
            .arg(sourceLang.isEmpty() ? QString::fromLocal8Bit("自动") : sourceLang)
            .arg(ui.comboTargetLang->currentText()), 5000);
    } else {
        ui.textEditTarget->setPlainText("");
        ui.statusBar->showMessage(QString::fromLocal8Bit("翻译失败: ") + error, 5000);
        QMessageBox::warning(this, QString::fromLocal8Bit("翻译失败"), error);
    }
}