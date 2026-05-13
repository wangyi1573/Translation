#include "VolcengineTranslate.h"
#include <curl/curl.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")
#endif

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static std::string BinaryToHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    return oss.str();
}

static std::string HmacSha256Windows(const std::string& key, const std::string& data) {
    std::string result;
    result.resize(32);
    ULONG len = 32;

    BCryptOpenAlgorithmProvider(nullptr, BCRYPT_HMAC_SHA256_ALGORITHM, nullptr, 0);
    
    BCRYPT_KEY_HANDLE hKey = nullptr;
    BCryptImportKey(nullptr,
        BCryptSimpleByteHash ? BCryptSimpleByteHash : nullptr,
        nullptr, &hKey, nullptr, 0,
        (PUCHAR)key.data(), (ULONG)key.size(), 0);

    // Use HMAC directly
    BCryptCreateHash(nullptr, nullptr, nullptr, 0, nullptr, 0, 0);
    
    // For simplicity, compute SHA256 of (key||data) if key <= 64 bytes
    // Otherwise SHA256(SHA256(key)||data)
    std::string keyData = key;
    if (keyData.size() > 64) {
        unsigned char hash[32];
        SHA256((unsigned char*)keyData.c_str(), keyData.size(), hash);
        keyData.assign((char*)hash, 32);
    }
    
    // Pad key to 64 bytes
    if (keyData.size() < 64) {
        keyData.resize(64, 0);
    }

    unsigned char ipad[64], opad[64];
    for (int i = 0; i < 64; ++i) {
        ipad[i] = keyData[i] ^ 0x36;
        opad[i] = keyData[i] ^ 0x5c;
    }

    // Inner hash
    unsigned char innerData[64 + SHA256_DIGEST_LENGTH];
    memcpy(innerData, ipad, 64);
    SHA256((unsigned char*)data.c_str(), data.size(), innerData + 64);
    
    unsigned char innerHash[SHA256_DIGEST_LENGTH];
    SHA256(innerData, 64 + SHA256_DIGEST_LENGTH, innerHash);

    // Outer hash
    unsigned char outerData[64 + SHA256_DIGEST_LENGTH];
    memcpy(outerData, opad, 64);
    memcpy(outerData + 64, innerHash, SHA256_DIGEST_LENGTH);

    unsigned char finalHash[SHA256_DIGEST_LENGTH];
    SHA256(outerData, 64 + SHA256_DIGEST_LENGTH, finalHash);
    
    return std::string((char*)finalHash, 32);
}

static std::string Sha256Hex(const unsigned char* data, size_t len) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(data, len, hash);
    return BinaryToHex(hash, SHA256_DIGEST_LENGTH);
}

VolcengineTranslate::VolcengineTranslate(const TranslateConfig& cfg) : m_cfg(cfg) {
    if (m_cfg.region.empty()) m_cfg.region = "cn-north-1";
    if (m_cfg.service.empty()) m_cfg.service = "translate";
    if (m_cfg.sourceLang.empty()) m_cfg.sourceLang = "auto";
    if (m_cfg.targetLang.empty()) m_cfg.targetLang = "zh";
    if (m_cfg.timeout <= 0) m_cfg.timeout = 30;
    if (m_cfg.apiUrl.empty()) m_cfg.apiUrl = "https://translate.volcengineapi.com";
}

VolcengineTranslate::~VolcengineTranslate() {}

std::string VolcengineTranslate::GetISO8601Time() {
    time_t now = time(nullptr);
    char buf[32];
    struct tm tmUtc;
#ifdef _WIN32
    gmtime_s(&tmUtc, &now);
#else
    gmtime_r(&now, &tmUtc);
#endif
    strftime(buf, sizeof(buf), "%Y%m%dT%H%M%SZ", &tmUtc);
    return std::string(buf);
}

std::string VolcengineTranslate::GetCurrentDate() {
    time_t now = time(nullptr);
    char buf[16];
    struct tm tmUtc;
#ifdef _WIN32
    gmtime_s(&tmUtc, &now);
#else
    gmtime_r(&now, &tmUtc);
#endif
    strftime(buf, sizeof(buf), "%Y%m%d", &tmUtc);
    return std::string(buf);
}

std::string VolcengineTranslate::Sha256Hex(const std::string& data) {
    return ::Sha256Hex((const unsigned char*)data.c_str(), data.size());
}

std::string VolcengineTranslate::HmacSha256(const std::string& key, const std::string& data) {
    return HmacSha256Windows(key, data);
}

std::string VolcengineTranslate::DeriveSigningKey(const std::string& date) {
    std::string kDate = HmacSha256("AWS4" + m_cfg.accessKeySecret, date);
    std::string kRegion = HmacSha256(kDate, m_cfg.region);
    std::string kService = HmacSha256(kRegion, m_cfg.service);
    return HmacSha256(kService, "request");
}

std::string VolcengineTranslate::UrlEncode(const std::string& value) {
    std::ostringstream oss;
    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            oss << c;
        } else {
            oss << '%' << std::uppercase
                << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
    }
    return oss.str();
}

std::string VolcengineTranslate::BuildAuthorization(const std::string& method,
                                                    const std::string& action,
                                                    const std::string& body) {
    std::string date = GetCurrentDate();
    std::string datetime = GetISO8601Time();
    std::string host = "translate.volcengineapi.com";
    std::string uri = "/";
    std::string queryStr = "Action=" + action + "&Version=2020-06-01";
    std::string contentSha256 = Sha256Hex(body);

    std::ostringstream canonicalSS;
    canonicalSS << method << "\n"
                << uri << "\n"
                << queryStr << "\n"
                << "content-type:application/json\n"
                << "host:" << host << "\n"
                << "x-content-sha256:" << contentSha256 << "\n"
                << "x-date:" << datetime << "\n"
                << "\n"
                << "content-type;host;x-content-sha256;x-date\n"
                << contentSha256;

    std::string hashedCanonicalRequest = Sha256Hex(canonicalSS.str());
    std::string credentialScope = date + "/" + m_cfg.region + "/" + m_cfg.service + "/request";

    std::ostringstream stsSS;
    stsSS << "HMAC-SHA256\n"
          << datetime << "\n"
          << credentialScope << "\n"
          << hashedCanonicalRequest;

    std::string signature = BinaryToHex(
        (const unsigned char*)HmacSha256(DeriveSigningKey(date), stsSS.str()).c_str(), 32);

    std::ostringstream authSS;
    authSS << "HMAC-SHA256 "
           << "Credential=" << m_cfg.accessKeyId << "/" << credentialScope << ", "
           << "SignedHeaders=content-type;host;x-content-sha256;x-date, "
           << "Signature=" << signature;

    return authSS.str();
}

std::string VolcengineTranslate::HttpPost(const std::string& url, const std::string& body,
                                          const std::map<std::string, std::string>& extraHeaders,
                                          std::string& errMsg) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        errMsg = "curl_easy_init failed";
        return "";
    }

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    for (const auto& h : extraHeaders) {
        headers = curl_slist_append(headers, (h.first + ": " + h.second).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)m_cfg.timeout);

    CURLcode res = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        errMsg = curl_easy_strerror(res);
        return "";
    }

    if (httpCode >= 400) {
        errMsg = "HTTP " + std::to_string(httpCode) + ": " + response;
        return "";
    }

    return response;
}

std::string VolcengineTranslate::Translate(const std::string& text, std::string& errMsg) {
    if (text.empty()) {
        errMsg = "Text is empty";
        return "";
    }

    std::ostringstream bodySS;
    bodySS << "{\"TargetLanguage\":\"" << m_cfg.targetLang << "\","
           << "\"TextList\":[\"" << text << "\"]";
    if (!m_cfg.sourceLang.empty() && m_cfg.sourceLang != "auto") {
        bodySS << ",\"SourceLanguage\":\"" << m_cfg.sourceLang << "\"";
    }
    bodySS << "}";
    std::string body = bodySS.str();

    std::string datetime = GetISO8601Time();
    std::string host = "translate.volcengineapi.com";
    std::string contentSha256 = Sha256Hex(body);

    std::map<std::string, std::string> extraHeaders;
    extraHeaders["Host"] = host;
    extraHeaders["X-Date"] = datetime;
    extraHeaders["X-Content-Sha256"] = contentSha256;
    extraHeaders["Authorization"] = BuildAuthorization("POST", "TranslateText", body);

    std::string url = m_cfg.apiUrl + "/?Action=TranslateText&Version=2020-06-01";
    std::string response = HttpPost(url, body, extraHeaders, errMsg);
    if (response.empty()) return "";

    size_t translationPos = response.find("\"Translation\"");
    if (translationPos == std::string::npos) {
        size_t msgPos = response.find("\"Message\"");
        if (msgPos != std::string::npos) {
            size_t start = response.find('"', msgPos + 9) + 1;
            size_t end = response.find('"', start);
            errMsg = response.substr(start, end - start);
        } else {
            errMsg = "Failed to parse: " + response.substr(0, 200);
        }
        return "";
    }

    size_t start = response.find('"', translationPos + 13) + 1;
    size_t end = response.find('"', start);
    std::string translated = response.substr(start, end - start);

    size_t detectedPos = response.find("\"DetectedSourceLanguage\"");
    if (detectedPos != std::string::npos) {
        size_t s = response.find('"', detectedPos + 24) + 1;
        size_t e = response.find('"', s);
        m_lastSourceLang = response.substr(s, e - s);
    }

    return translated;
}

std::string VolcengineTranslate::DetectLanguage(const std::string& text, std::string& errMsg) {
    if (text.empty()) {
        errMsg = "Text is empty";
        return "";
    }

    std::string body = "{\"Text\":\"" + text + "\"}";
    std::string datetime = GetISO8601Time();
    std::string host = "translate.volcengineapi.com";
    std::string contentSha256 = Sha256Hex(body);

    std::map<std::string, std::string> extraHeaders;
    extraHeaders["Host"] = host;
    extraHeaders["X-Date"] = datetime;
    extraHeaders["X-Content-Sha256"] = contentSha256;
    extraHeaders["Authorization"] = BuildAuthorization("POST", "LangDetect", body);

    std::string url = m_cfg.apiUrl + "/?Action=LangDetect&Version=2020-06-01";
    std::string response = HttpPost(url, body, extraHeaders, errMsg);
    if (response.empty()) return "";

    size_t pos = response.find("\"DetectedLanguage\"");
    if (pos == std::string::npos) {
        errMsg = "Failed to detect: " + response.substr(0, 200);
        return "";
    }

    size_t start = response.find('"', pos + 19) + 1;
    size_t end = response.find('"', start);
    return response.substr(start, end - start);
}

bool VolcengineTranslate::TestConnection(std::string& errMsg) {
    std::string result = Translate("hello", errMsg);
    return !result.empty();
}