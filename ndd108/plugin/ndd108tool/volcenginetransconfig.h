#pragma once
#include <QDialog>
#include <QString>
#include "ui_volcenginetransconfig.h"

class VolcengineApi;

class VolcengineTransConfig : public QDialog {
    Q_OBJECT

public:
    explicit VolcengineTransConfig(QWidget* parent);
    ~VolcengineTransConfig();

    void loadConfig();
    void saveConfig();

signals:
    void configChanged(const QString& accessKeyId, const QString& accessKeySecret);

private slots:
    void onTestConnection();
    void onSave();
    void onInputChanged();

private:
    Ui_VolcengineTransConfig ui;
    QString m_accessKeyId;
    QString m_accessKeySecret;
};