#ifndef CONFIGSTORAGE_H
#define CONFIGSTORAGE_H

#include <QString>
#include <QSettings>
#include <QCoreApplication>

/**
 * @brief 配置存储类
 * 负责用户配置的持久化存储和读取
 */
class ConfigStorage {
public:
    /**
     * @brief 构造函数
     * @param configFile 配置文件路径，默认为空（使用默认路径）
     */
    ConfigStorage(const QString& configFile = "");
    
    /**
     * @brief 析构函数
     */
    ~ConfigStorage();
    
    /**
     * @brief 保存火山翻译API的访问密钥
     * @param accessKeyId Access Key ID
     * @param accessKeySecret Access Key Secret
     * @return 是否保存成功
     */
    bool saveApiKeys(const QString& accessKeyId, const QString& accessKeySecret);
    
    /**
     * @brief 获取Access Key ID
     * @return Access Key ID
     */
    QString getAccessKeyId() const;
    
    /**
     * @brief 获取Access Key Secret
     * @return Access Key Secret
     */
    QString getAccessKeySecret() const;
    
    /**
     * @brief 保存源语言设置
     * @param sourceLanguage 源语言代码
     * @return 是否保存成功
     */
    bool saveSourceLanguage(const QString& sourceLanguage);
    
    /**
     * @brief 获取源语言设置
     * @return 源语言代码
     */
    QString getSourceLanguage() const;
    
    /**
     * @brief 保存目标语言设置
     * @param targetLanguage 目标语言代码
     * @return 是否保存成功
     */
    bool saveTargetLanguage(const QString& targetLanguage);
    
    /**
     * @brief 获取目标语言设置
     * @return 目标语言代码
     */
    QString getTargetLanguage() const;
    
    /**
     * @brief 保存自动检测语言设置
     * @param autoDetect 是否自动检测语言
     * @return 是否保存成功
     */
    bool saveAutoDetectLanguage(bool autoDetect);
    
    /**
     * @brief 获取自动检测语言设置
     * @return 是否自动检测语言
     */
    bool getAutoDetectLanguage() const;
    
    /**
     * @brief 保存请求超时设置
     * @param timeout 超时时间（毫秒）
     * @return 是否保存成功
     */
    bool saveTimeout(int timeout);
    
    /**
     * @brief 获取请求超时设置
     * @return 超时时间（毫秒）
     */
    int getTimeout() const;
    
    /**
     * @brief 重置所有配置为默认值
     * @return 是否重置成功
     */
    bool resetToDefaults();
    
    /**
     * @brief 检查配置文件是否存在且有效
     * @return 配置是否有效
     */
    bool isConfigValid() const;

private:
    QSettings* m_settings;    // Qt设置对象，用于读写配置
    QString m_configFile;     // 配置文件路径
    
    /**
     * @brief 获取默认配置文件路径
     * @return 默认配置文件路径
     */
    QString getDefaultConfigPath() const;
    
    /**
     * @brief 初始化默认配置
     */
    void initializeDefaults();
};

#endif // CONFIGSTORAGE_H