#include "volcengineapi.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QNetworkRequest>
#include <QDebug>

VolcengineApi::VolcengineApi(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_region("cn-north-1")
{
}

VolcengineApi::~VolcengineApi() {
    cancelAll();
}

void VolcengineApi::setCredentials(const QString& accessKeyId, const QString& accessKeySecret) {
    m_accessKeyId = accessKeyId;
    m_accessKeySecret = accessKeySecret;
}

void VolcengineApi::setRegion(const QString& region) {
    m_region = region;
}

QString VolcengineApi::sha256Hex(const QByteArray& data) {
    return QString::fromUtf8(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

QString VolcengineApi::hmacSha256Hex(const QString& key, const QString& data) {
    QByteArray keyBytes = key.toUtf8();
    int blockSize = 64;

    if (keyBytes.size() > blockSize) {
        keyBytes = QCryptographicHash::hash(keyBytes, QCryptographicHash::Sha256);
    }
    if (keyBytes.size() < blockSize) {
        keyBytes.append(QByteArray(blockSize - keyBytes.size(), 0));
    }

    QByteArray iPad(blockSize, 0x36);
    QByteArray oPad(blockSize, 0x5c);
    for (int i = 0; i < blockSize; ++i) {
        iPad[i] = iPad[i] ^ keyBytes[i];
        oPad[i] = oPad[i] ^ keyBytes[i];
    }

    QByteArray innerHash = QCryptographicHash::hash(iPad + data.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromUtf8(QCryptographicHash::hash(oPad + innerHash, QCryptographicHash::Sha256).toHex());
}

QString VolcengineApi::getCurrentTimeISO8601() {
    return QDateTime::currentDateTimeUtc().toString("yyyyMMddTHHmmssZ");
}

QString VolcengineApi::getCurrentDate() {
    return QDateTime::currentDateTimeUtc().toString("yyyyMMdd");
}

QString VolcengineApi::generateAuthorization(const QString& method, const QString& canonicalUri,
                                             const QMap<QString, QString>& headers, const QByteArray& body) {
    Q_UNUSED(method);
    Q_UNUSED(canonicalUri);
    Q_UNUSED(headers);

    QString datetime = getCurrentTimeISO8601();
    QString date = getCurrentDate();
    QString host = "translate.volcengineapi.com";
    QString contentSha256 = sha256Hex(body);

    QString canonicalHeaders = QString("content-type:application/json\nhost:%1\nx-date:%2\n").arg(host).arg(datetime);
    QString signedHeaders = "content-type;host;x-date";

    QString canonicalRequest = QString("%1\n%2\n%3\n%4\n%5\n%6")
        .arg("POST")
        .arg("/")
        .arg("")
        .arg(canonicalHeaders)
        .arg(signedHeaders)
        .arg(contentSha256);

    QString credentialScope = QString("%1/%2/translate/request").arg(date).arg(m_region);
    QString hashedCanonicalRequest = sha256Hex(canonicalRequest.toUtf8());

    QString stringToSign = QString("HMAC-SHA256\n%1\n%2\n%3")
        .arg(datetime)
        .arg(credentialScope)
        .arg(hashedCanonicalRequest);

    QString kDate = hmacSha256Hex("AWS4" + m_accessKeySecret, date);
    QString kRegion = hmacSha256Hex(kDate, m_region);
    QString kService = hmacSha256Hex(kRegion, "translate");
    QString kSigning = hmacSha256Hex(kService, "request");
    QString signature = hmacSha256Hex(kSigning, stringToSign);

    return QString("HMAC-SHA256 Credential=%1/%2, SignedHeaders=%3, Signature=%4")
        .arg(m_accessKeyId)
        .arg(credentialScope)
        .arg(signedHeaders)
        .arg(signature);
}

void VolcengineApi::sendSignedRequest(const QString& url, const QByteArray& body) {
    if (m_accessKeyId.isEmpty() || m_accessKeySecret.isEmpty()) {
        if (m_pendingAction == "translate") {
            emit translateFinished(false, "", "", QString::fromLocal8Bit("请先配置API密钥"));
        } else if (m_pendingAction == "detect") {
            emit detectFinished(false, "", QString::fromLocal8Bit("请先配置API密钥"));
        } else {
            emit connectionTestFinished(false, QString::fromLocal8Bit("请先配置API密钥"));
        }
        return;
    }

    QString datetime = getCurrentTimeISO8601();
    QString host = "translate.volcengineapi.com";
    QString contentSha256 = sha256Hex(body);
    QString auth = generateAuthorization("POST", "/", QMap<QString, QString>(), body);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Host", host.toUtf8());
    request.setRawHeader("X-Date", datetime.toUtf8());
    request.setRawHeader("X-Content-Sha256", contentSha256.toUtf8());
    request.setRawHeader("Authorization", auth.toUtf8());

    if (m_currentReply) {
        m_currentReply->deleteLater();
    }
    m_currentReply = m_nam->post(request, body);
    connect(m_currentReply, &QNetworkReply::finished, this, &VolcengineApi::onReplyFinished);
}

static QByteArray escapeJsonString(const QString& str) {
    QByteArray result;
    result.reserve(str.size() * 2);
    for (const QChar& c : str) {
        switch (c.unicode()) {
        case '"':  result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        default:
            if (c.unicode() < 0x80) {
                result += (char)c.unicode();
            } else {
                result += QString(c).toUtf8();
            }
            break;
        }
    }
    return result;
}

void VolcengineApi::translate(const QString& text, const QString& sourceLang, const QString& targetLang) {
    m_pendingAction = "translate";
    m_pendingSourceLang = sourceLang;
    m_pendingTargetLang = targetLang;

    QByteArray body = QByteArray("{\"TargetLanguage\":\"") + escapeJsonString(targetLang) + "\",\"TextList\":[\"" + escapeJsonString(text) + "\"]";
    if (!sourceLang.isEmpty() && sourceLang != "auto") {
        body += ",\"SourceLanguage\":\"" + escapeJsonString(sourceLang) + "\"";
    }
    body += "]}";

    QString url = "https://translate.volcengineapi.com/?Action=TranslateText&Version=2020-06-01";
    sendSignedRequest(url, body);
}

void VolcengineApi::detectLanguage(const QString& text) {
    m_pendingAction = "detect";
    QByteArray body = QByteArray("{\"Text\":\"") + escapeJsonString(text) + "\"}";
    QString url = "https://translate.volcengineapi.com/?Action=LangDetect&Version=2020-06-01";
    sendSignedRequest(url, body);
}

void VolcengineApi::testConnection() {
    m_pendingAction = "test";
    m_pendingSourceLang = "";
    m_pendingTargetLang = "zh";

    QByteArray body = QByteArray("{\"TargetLanguage\":\"zh\",\"TextList\":[\"hello\"]}");

    QString url = "https://translate.volcengineapi.com/?Action=TranslateText&Version=2020-06-01";
    sendSignedRequest(url, body);
}

void VolcengineApi::cancelAll() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void VolcengineApi::onReplyFinished() {
    if (!m_currentReply) return;

    QNetworkReply* reply = m_currentReply;
    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = reply->errorString();
        if (m_pendingAction == "translate") {
            emit translateFinished(false, "", "", errorMsg);
        } else if (m_pendingAction == "detect") {
            emit detectFinished(false, "", errorMsg);
        } else {
            emit connectionTestFinished(false, errorMsg);
        }
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (m_pendingAction == "test") {
        // Test connection: parse response to check for API errors, emit connectionTestFinished only
        QString jsonStr = QString::fromUtf8(data);
        if (jsonStr.indexOf("\"Error\"") >= 0) {
            int msgStart = jsonStr.indexOf("\"Message\"");
            QString errorMsg = QString::fromLocal8Bit("连接失败");
            if (msgStart >= 0) {
                int q1 = jsonStr.indexOf('"', msgStart + 9) + 1;
                int q2 = jsonStr.indexOf('"', q1);
                if (q1 > 0 && q2 > q1) {
                    errorMsg = jsonStr.mid(q1, q2 - q1);
                }
            }
            emit connectionTestFinished(false, errorMsg);
        } else {
            emit connectionTestFinished(true, "");
        }
    } else if (m_pendingAction == "translate") {
        parseTranslateResponse(data);
    } else if (m_pendingAction == "detect") {
        parseDetectResponse(data);
    }
}

void VolcengineApi::parseTranslateResponse(const QByteArray& data) {
    QString jsonStr = QString::fromUtf8(data);

    int errorPos = jsonStr.indexOf("\"Error\"");
    if (errorPos >= 0) {
        int msgStart = jsonStr.indexOf("\"Message\"", errorPos);
        if (msgStart >= 0) {
            int q1 = jsonStr.indexOf('"', msgStart + 9) + 1;
            int q2 = jsonStr.indexOf('"', q1);
            emit translateFinished(false, "", "", jsonStr.mid(q1, q2 - q1));
            return;
        }
        emit translateFinished(false, "", "", QString::fromLocal8Bit("API返回错误"));
        return;
    }

    int transPos = jsonStr.indexOf("\"Translation\"");
    if (transPos < 0) {
        emit translateFinished(false, "", "", QString::fromLocal8Bit("无法解析翻译结果"));
        return;
    }

    int q1 = jsonStr.indexOf('"', transPos + 13) + 1;
    int q2 = jsonStr.indexOf('"', q1);
    QString translated = jsonStr.mid(q1, q2 - q1);

    QString detectedLang = m_pendingSourceLang;
    int detectedPos = jsonStr.indexOf("\"DetectedSourceLanguage\"");
    if (detectedPos >= 0) {
        int sq1 = jsonStr.indexOf('"', detectedPos + 23) + 1;
        int sq2 = jsonStr.indexOf('"', sq1);
        detectedLang = jsonStr.mid(sq1, sq2 - sq1);
    }

    emit translateFinished(true, translated, detectedLang, "");
}

void VolcengineApi::parseDetectResponse(const QByteArray& data) {
    QString jsonStr = QString::fromUtf8(data);

    if (jsonStr.indexOf("\"Error\"") >= 0) {
        emit detectFinished(false, "", QString::fromLocal8Bit("语言检测失败"));
        return;
    }

    int pos = jsonStr.indexOf("\"DetectedLanguage\"");
    if (pos < 0) {
        emit detectFinished(false, "", QString::fromLocal8Bit("无法解析语言检测结果"));
        return;
    }

    int q1 = jsonStr.indexOf('"', pos + 19) + 1;
    int q2 = jsonStr.indexOf('"', q1);
    emit detectFinished(true, jsonStr.mid(q1, q2 - q1), "");
}