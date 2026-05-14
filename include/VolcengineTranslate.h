#pragma once

#include <string>
#include <vector>
#include <map>
#include <ctime>

struct TranslateConfig {
    std::string accessKeyId;
    std::string accessKeySecret;
    std::string apiUrl;       // e.g. "https://translate.volcengineapi.com"
    std::string region;       // default: "cn-north-1"
    std::string service;      // default: "translate"
    std::string sourceLang;   // default: "auto"
    std::string targetLang;   // default: "zh"
    int timeout;              // seconds, default: 30
};

class VolcengineTranslate {
public:
    VolcengineTranslate(const TranslateConfig& cfg);
    ~VolcengineTranslate();

    std::string Translate(const std::string& text, std::string& errMsg);
    std::string DetectLanguage(const std::string& text, std::string& errMsg);
    bool TestConnection(std::string& errMsg);

private:
    TranslateConfig m_cfg;
    std::string m_lastSourceLang;

    // HMAC-SHA256 V4 signing
    std::string HmacSha256(const std::string& key, const std::string& data);
    std::string Sha256Hex(const std::string& data);
    std::string DeriveSigningKey(const std::string& date);
    std::string GetISO8601Time();
    std::string GetCurrentDate();
    std::string UrlEncode(const std::string& value);
    std::string BuildAuthorization(const std::string& method, const std::string& action,
                                    const std::string& body);

    // HTTP
    std::string HttpPost(const std::string& url, const std::string& body,
                         const std::map<std::string, std::string>& extraHeaders,
                         std::string& errMsg);
};