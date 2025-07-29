/**
 * @file SessionManager.h
 * @author unravel
 * @date 2025-07-28
 * @framework Qt 6.x, C++17 Standard
 * @technology 会话管理, UUID生成, 定时器, 持久化存储
 * @task UnravelChat第二阶段 - 会话管理器
 */

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QSettings>
#include <QtCore/QUuid>
#include <QtCore/QStandardPaths>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <memory>

enum class SessionState {
    Invalid,
    Active,
    Expired,
    Suspended,
    Terminated
};

struct SessionInfo {
    QString sessionId;
    QString username;
    QDateTime createdAt;
    QDateTime lastAccessAt;
    QDateTime expiresAt;
    QString clientInfo;
    bool rememberMe;
    SessionState state;
    
    SessionInfo() : rememberMe(false), state(SessionState::Invalid) {}
    
    bool isValid() const {
        return !sessionId.isEmpty() && !username.isEmpty() && 
               state == SessionState::Active && 
               QDateTime::currentDateTime() < expiresAt;
    }
    
    bool isExpired() const {
        return QDateTime::currentDateTime() >= expiresAt;
    }
    
    qint64 getRemainingTime() const {
        return QDateTime::currentDateTime().secsTo(expiresAt);
    }
    
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["sessionId"] = sessionId;
        obj["username"] = username;
        obj["createdAt"] = createdAt.toString(Qt::ISODate);
        obj["lastAccessAt"] = lastAccessAt.toString(Qt::ISODate);
        obj["expiresAt"] = expiresAt.toString(Qt::ISODate);
        obj["clientInfo"] = clientInfo;
        obj["rememberMe"] = rememberMe;
        obj["state"] = static_cast<int>(state);
        return obj;
    }
    
    static SessionInfo fromJson(const QJsonObject& obj) {
        SessionInfo info;
        info.sessionId = obj["sessionId"].toString();
        info.username = obj["username"].toString();
        info.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        info.lastAccessAt = QDateTime::fromString(obj["lastAccessAt"].toString(), Qt::ISODate);
        info.expiresAt = QDateTime::fromString(obj["expiresAt"].toString(), Qt::ISODate);
        info.clientInfo = obj["clientInfo"].toString();
        info.rememberMe = obj["rememberMe"].toBool();
        info.state = static_cast<SessionState>(obj["state"].toInt());
        return info;
    }
};

struct SessionConfig {
    int defaultTimeoutMinutes = 120;
    int rememberMeDays = 30;
    int maxConcurrentSessions = 3;
    int sessionCheckIntervalSeconds = 60;
    bool allowMultipleSessions = true;
    bool persistSessions = true;
    
    static SessionConfig defaultConfig() {
        return SessionConfig{};
    }
};

class SessionManager : public QObject {
    Q_OBJECT

private:
    SessionConfig config_;
    SessionInfo currentSession_;
    std::unique_ptr<QSettings> sessionStorage_;
    QString storagePath_;
    
    QTimer* sessionCheckTimer_;
    QTimer* cleanupTimer_;
    bool isInitialized_;

public:
    explicit SessionManager(QObject* parent = nullptr, 
                           const SessionConfig& config = SessionConfig::defaultConfig());
    ~SessionManager();
    
    bool initialize();

    // 会话创建和管理
    QString createSession(const QString& username, bool rememberMe = false, const QString& clientInfo = "");
    bool validateSession(const QString& sessionId);
    bool refreshSession(const QString& sessionId = "");
    bool destroySession(const QString& sessionId = "");
    int destroyUserSessions(const QString& username);

    // 会话查询
    SessionInfo getCurrentSession() const { return currentSession_; }
    SessionInfo getSessionInfo(const QString& sessionId) const;
    QString getCurrentUsername() const { return currentSession_.username; }
    QString getCurrentSessionId() const { return currentSession_.sessionId; }
    bool hasActiveSession() const;
    qint64 getSessionRemainingTime(const QString& sessionId = "") const;

    // 会话管理
    bool extendSession(const QString& sessionId = "", int minutes = 60);
    QStringList getUserActiveSessions(const QString& username) const;
    int getActiveSessionCount() const;

    // 自动登录管理
    QString setAutoLoginToken(const QString& username);
    QString validateAutoLoginToken(const QString& token);
    bool clearAutoLoginToken(const QString& username);

    // 配置管理
    void setSessionConfig(const SessionConfig& config);
    const SessionConfig& getSessionConfig() const { return config_; }

    // 实用工具
    static QString generateSessionId();
    static QString generateSecureToken(const QString& data = "");
    static QString sessionStateToString(SessionState state);

private:
    bool loadPersistedSessions();
    bool saveSessionToPersistentStorage(const SessionInfo& sessionInfo);
    bool removeSessionFromPersistentStorage(const QString& sessionId);
    int cleanupExpiredSessions();
    bool checkSessionLimits(const QString& username);
    QDateTime calculateExpiryTime(bool rememberMe) const;
    QString generateClientInfo() const;

private slots:
    void onSessionCheck();
    void onCleanupTimer();

signals:
    void sessionCreated(const QString& sessionId, const QString& username);
    void sessionDestroyed(const QString& sessionId, const QString& username);
    void sessionExpired(const QString& sessionId, const QString& username);
    void sessionRefreshed(const QString& sessionId);
    void sessionError(const QString& operation, const QString& error);
};

#endif // SESSIONMANAGER_H

