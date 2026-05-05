#include "ConfigStorage.h"
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QStandardPaths>

ConfigStorage::ConfigStorage(const QString& configFile)
    : m_configFile(configFile)
{
    // 如果未指定配置文件路径，使用默认路径
    if (m_configFile.isEmpty()) {
        m_configFile = getDefaultConfigPath();
    }
    
    // 确保配置文件目录存在
    QFileInfo configFileInfo(m_configFile);
    QDir configDir = configFileInfo.dir();
    if (!configDir.exists()) {
        configDir.mkpath(configDir.absolutePath());
    }
    
    // 初始化QSettings
    m_settings = new QSettings(m_configFile, QSettings::IniFormat);
    
    // 初始化默认配置
    initializeDefaults();
}

ConfigStorage::~ConfigStorage()
{
    delete m_settings;
}

QString ConfigStorage::getDefaultConfigPath() const
{
    // 在不同平台上使用标准位置存储配置文件
    QString configDir;
    
#ifdef Q_OS_WIN
    configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else
    configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#endif
    
    return configDir + QDir::separator() + "VolcengineTranslation" + QDir::separator() + "config.ini";
}

void ConfigStorage::initializeDefaults()
{
    // 如果配置文件不存在或为空，设置默认值
    if (!m_settings->contains("api/accessKeyId")) {
        m_settings->setValue("api/accessKeyId", "");
    }
    
    if (!m_settings->contains("api/accessKeySecret")) {
        m_settings->setValue("api/accessKeySecret", "");
    }
    
    if (!m_settings->contains("translation/sourceLanguage")) {
        m_settings->setValue("translation/sourceLanguage", "auto");
    }
    
    if (!m_settings->contains("translation/targetLanguage")) {
        m_settings->setValue("translation/targetLanguage", "zh");
    }
    
    if (!m_settings->contains("translation/autoDetectLanguage")) {
        m_settings->setValue("translation/autoDetectLanguage", true);
    }
    
    if (!m_settings->contains("network/timeout")) {
        m_settings->setValue("network/timeout", 10000); // 默认10秒超时
    }
    
    m_settings->sync();
}

bool ConfigStorage::saveApiKeys(const QString& accessKeyId, const QString& accessKeySecret)
{
    m_settings->setValue("api/accessKeyId", accessKeyId);
    m_settings->setValue("api/accessKeySecret", accessKeySecret);
    return m_settings->sync();
}

QString ConfigStorage::getAccessKeyId() const
{
    return m_settings->value("api/accessKeyId").toString();
}

QString ConfigStorage::getAccessKeySecret() const
{
    return m_settings->value("api/accessKeySecret").toString();
}

bool ConfigStorage::saveSourceLanguage(const QString& sourceLanguage)
{
    m_settings->setValue("translation/sourceLanguage", sourceLanguage);
    return m_settings->sync();
}

QString ConfigStorage::getSourceLanguage() const
{
    return m_settings->value("translation/sourceLanguage").toString();
}

bool ConfigStorage::saveTargetLanguage(const QString& targetLanguage)
{
    m_settings->setValue("translation/targetLanguage", targetLanguage);
    return m_settings->sync();
}

QString ConfigStorage::getTargetLanguage() const
{
    return m_settings->value("translation/targetLanguage").toString();
}

bool ConfigStorage::saveAutoDetectLanguage(bool autoDetect)
{
    m_settings->setValue("translation/autoDetectLanguage", autoDetect);
    return m_settings->sync();
}

bool ConfigStorage::getAutoDetectLanguage() const
{
    return m_settings->value("translation/autoDetectLanguage").toBool();
}

bool ConfigStorage::saveTimeout(int timeout)
{
    m_settings->setValue("network/timeout", timeout);
    return m_settings->sync();
}

int ConfigStorage::getTimeout() const
{
    return m_settings->value("network/timeout").toInt();
}

bool ConfigStorage::resetToDefaults()
{
    m_settings->clear();
    initializeDefaults();
    return m_settings->sync();
}

bool ConfigStorage::isConfigValid() const
{
    // 检查必要的配置项是否存在
    return m_settings->contains("api/accessKeyId") &&
           m_settings->contains("api/accessKeySecret") &&
           m_settings->contains("translation/sourceLanguage") &&
           m_settings->contains("translation/targetLanguage") &&
           m_settings->contains("translation/autoDetectLanguage") &&
           m_settings->contains("network/timeout");
}