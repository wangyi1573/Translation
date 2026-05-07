#include "../include/PluginCore/ConfigManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>

ConfigManager::ConfigManager()
    : m_settings(nullptr)
{
    m_configFile = getDefaultConfigPath();
    m_settings = new QSettings(m_configFile, QSettings::IniFormat);
    initializeDefaults();
}

ConfigManager::~ConfigManager()
{
    if (m_settings) {
        delete m_settings;
    }
}

QString ConfigManager::getDefaultConfigPath() const
{
    QString configDir = QCoreApplication::applicationDirPath() + "/plugins/config";
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return configDir + "/volcengine_translation.ini";
}

void ConfigManager::initializeDefaults()
{
    if (!m_settings->contains("timeout")) {
        m_settings->setValue("timeout", 30000);
    }
    if (!m_settings->contains("autoDetectLanguage")) {
        m_settings->setValue("autoDetectLanguage", true);
    }
    if (!m_settings->contains("sourceLanguage")) {
        m_settings->setValue("sourceLanguage", "auto");
    }
    if (!m_settings->contains("targetLanguage")) {
        m_settings->setValue("targetLanguage", "zh");
    }
}

bool ConfigManager::saveApiKeys(const QString& accessKeyId, const QString& accessKeySecret)
{
    m_settings->setValue("accessKeyId", accessKeyId);
    m_settings->setValue("accessKeySecret", encryptSecret(accessKeySecret));
    m_settings->sync();
    return m_settings->status() == QSettings::NoError;
}

QString ConfigManager::getAccessKeyId() const
{
    return m_settings->value("accessKeyId", "").toString();
}

QString ConfigManager::getAccessKeySecret() const
{
    QString encrypted = m_settings->value("accessKeySecret", "").toString();
    if (encrypted.isEmpty()) {
        return "";
    }
    return decryptSecret(encrypted);
}

bool ConfigManager::saveSourceLanguage(const QString& sourceLanguage)
{
    m_settings->setValue("sourceLanguage", sourceLanguage);
    m_settings->sync();
    return m_settings->status() == QSettings::NoError;
}

QString ConfigManager::getSourceLanguage() const
{
    return m_settings->value("sourceLanguage", "auto").toString();
}

bool ConfigManager::saveTargetLanguage(const QString& targetLanguage)
{
    m_settings->setValue("targetLanguage", targetLanguage);
    m_settings->sync();
    return m_settings->status() == QSettings::NoError;
}

QString ConfigManager::getTargetLanguage() const
{
    return m_settings->value("targetLanguage", "zh").toString();
}

bool ConfigManager::saveAutoDetectLanguage(bool autoDetect)
{
    m_settings->setValue("autoDetectLanguage", autoDetect);
    m_settings->sync();
    return m_settings->status() == QSettings::NoError;
}

bool ConfigManager::getAutoDetectLanguage() const
{
    return m_settings->value("autoDetectLanguage", true).toBool();
}

bool ConfigManager::saveTimeout(int timeout)
{
    m_settings->setValue("timeout", timeout);
    m_settings->sync();
    return m_settings->status() == QSettings::NoError;
}

int ConfigManager::getTimeout() const
{
    return m_settings->value("timeout", 30000).toInt();
}

bool ConfigManager::resetToDefaults()
{
    m_settings->clear();
    initializeDefaults();
    m_settings->sync();
    return m_settings->status() == QSettings::NoError;
}

bool ConfigManager::isConfigValid() const
{
    return !getAccessKeyId().isEmpty() && !getAccessKeySecret().isEmpty();
}

QString ConfigManager::encryptSecret(const QString& secret) const
{
    if (secret.isEmpty()) {
        return "";
    }
    
    // 简单的XOR加密（实际生产环境应使用更强的加密）
    QByteArray key = "VolcengineTranslationPlugin2024";
    QByteArray data = secret.toUtf8();
    QByteArray result;
    
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ key[i % key.size()]);
    }
    
    return result.toBase64();
}

QString ConfigManager::decryptSecret(const QString& encryptedSecret) const
{
    if (encryptedSecret.isEmpty()) {
        return "";
    }
    
    QByteArray key = "VolcengineTranslationPlugin2024";
    QByteArray data = QByteArray::fromBase64(encryptedSecret.toUtf8());
    QByteArray result;
    
    for (int i = 0; i < data.size(); ++i) {
        result.append(data[i] ^ key[i % key.size()]);
    }
    
    return QString::fromUtf8(result);
}