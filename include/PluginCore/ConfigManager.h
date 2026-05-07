#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <QString>
#include <QSettings>

/**
 * @brief 配置管理类
 * 负责用户配置的持久化存储和读取
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    bool saveApiKeys(const QString& accessKeyId, const QString& accessKeySecret);
    QString getAccessKeyId() const;
    QString getAccessKeySecret() const;
    
    bool saveSourceLanguage(const QString& sourceLanguage);
    QString getSourceLanguage() const;
    
    bool saveTargetLanguage(const QString& targetLanguage);
    QString getTargetLanguage() const;
    
    bool saveAutoDetectLanguage(bool autoDetect);
    bool getAutoDetectLanguage() const;
    
    bool saveTimeout(int timeout);
    int getTimeout() const;
    
    bool resetToDefaults();
    bool isConfigValid() const;

private:
    void initializeDefaults();
    QString getDefaultConfigPath() const;
    QString encryptSecret(const QString& secret) const;
    QString decryptSecret(const QString& encryptedSecret) const;
    
    QSettings* m_settings;
    QString m_configFile;
};

#endif // CONFIG_MANAGER_H