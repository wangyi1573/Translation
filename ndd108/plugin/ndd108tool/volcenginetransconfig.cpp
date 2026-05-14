#include "volcenginetransconfig.h"
#include "volcengineapi.h"
#include <QSettings>
#include <QMessageBox>

static const char* CONFIG_KEY_ID = "VolcengineTranslate/AccessKeyId";
static const char* CONFIG_KEY_SECRET = "VolcengineTranslate/AccessKeySecret";

VolcengineTransConfig::VolcengineTransConfig(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    setWindowTitle(QString::fromLocal8Bit("火山翻译配置"));

    loadConfig();

    connect(ui.btnTest, &QPushButton::clicked, this, &VolcengineTransConfig::onTestConnection);
    connect(ui.btnSave, &QPushButton::clicked, this, &VolcengineTransConfig::onSave);
    connect(ui.btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui.lineEditAccessKeyId, &QLineEdit::textChanged, this, &VolcengineTransConfig::onInputChanged);
    connect(ui.lineEditAccessKeySecret, &QLineEdit::textChanged, this, &VolcengineTransConfig::onInputChanged);
}

VolcengineTransConfig::~VolcengineTransConfig() {}

void VolcengineTransConfig::loadConfig() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Notepad--", "config");
    m_accessKeyId = settings.value(CONFIG_KEY_ID, "").toString();
    m_accessKeySecret = settings.value(CONFIG_KEY_SECRET, "").toString();

    ui.lineEditAccessKeyId->setText(m_accessKeyId);
    ui.lineEditAccessKeySecret->setText(m_accessKeySecret);
}

void VolcengineTransConfig::saveConfig() {
    m_accessKeyId = ui.lineEditAccessKeyId->text().trimmed();
    m_accessKeySecret = ui.lineEditAccessKeySecret->text().trimmed();

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Notepad--", "config");
    settings.setValue(CONFIG_KEY_ID, m_accessKeyId);
    settings.setValue(CONFIG_KEY_SECRET, m_accessKeySecret);
    settings.sync();
}

void VolcengineTransConfig::onTestConnection() {
    QString ak = ui.lineEditAccessKeyId->text().trimmed();
    QString sk = ui.lineEditAccessKeySecret->text().trimmed();

    if (ak.isEmpty() || sk.isEmpty()) {
        QMessageBox::warning(this, QString::fromLocal8Bit("提示"),
            QString::fromLocal8Bit("请输入完整的API密钥"));
        return;
    }

    ui.btnTest->setEnabled(false);
    ui.btnTest->setText(QString::fromLocal8Bit("测试中..."));

    VolcengineApi* api = new VolcengineApi(this);
    api->setCredentials(ak, sk);

    connect(api, &VolcengineApi::connectionTestFinished, this,
        [this, api](bool success, const QString& error) {
            ui.btnTest->setEnabled(true);
            ui.btnTest->setText(QString::fromLocal8Bit("测试连接"));
            if (success) {
                QMessageBox::information(this, QString::fromLocal8Bit("测试结果"),
                    QString::fromLocal8Bit("连接测试成功！"));
            } else {
                QMessageBox::warning(this, QString::fromLocal8Bit("测试结果"),
                    QString::fromLocal8Bit("连接失败: ") + error);
            }
            api->deleteLater();
        });

    api->testConnection();
}

void VolcengineTransConfig::onSave() {
    saveConfig();
    emit configChanged(m_accessKeyId, m_accessKeySecret);
    accept();
}

void VolcengineTransConfig::onInputChanged() {
    // Enable/disable save button based on input
    QString ak = ui.lineEditAccessKeyId->text().trimmed();
    QString sk = ui.lineEditAccessKeySecret->text().trimmed();
    ui.btnSave->setEnabled(!ak.isEmpty() && !sk.isEmpty());
}