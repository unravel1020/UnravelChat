/**
#include <QtCore/QCoreApplication>
 * @file SessionManager.cpp
 * @author unravel
 * @date 2025-07-28
 * @framework Qt 6.x, C++17 Standard
 * @technology 会话管理实现
 * @task UnravelChat第二阶段 - 会话管理器实现
 */

#include "../../include/auth/SessionManager.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QCryptographicHash>

SessionManager::SessionManager(QObject* parent, const SessionConfig& config)
    : QObject(parent)
    , config_(config)
    , isInitialized_(false)
{
    sessionCheckTimer_ = new QTimer(this);
    cleanupTimer_ = new QTimer(this);
    
    connect(sessionCheckTimer_, &QTimer::timeout, this, &SessionManager::onSessionCheck);
    connect(cleanupTimer_, &QTimer::timeout, this, &SessionManager::onCleanupTimer);
}

SessionManager::~SessionManager() {
    if (sessionCheckTimer_) {
        sessionCheckTimer_->stop();
    }
    if (cleanupTimer_) {
        cleanupTimer_->stop();
    }
    
    // 保存当前会话状态
    if (currentSession_.isValid() && config_.persistSessions) {
        saveSessionToPersistentStorage(currentSession_);
    }
}

bool SessionManager::initialize() {
    try {
        // 创建会话存储目录
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir;
        if (!dir.exists(appDataPath)) {
            if (!dir.mkpath(appDataPath)) {
                emit sessionError("Initialize", "Failed to create session directory");
                return false;
            }
        }
        
        // 设置会话存储路径
        storagePath_ = appDataPath + "/sessions.ini";
        
        if (config_.persistSessions) {
            sessionStorage_ = std::make_unique<QSettings>(storagePath_, QSettings::IniFormat);
            
            if (!sessionStorage_->isWritable()) {
                emit sessionError("Initialize", "Session storage is not writable");
                return false;
            }
            
            // 加载持久化会话
            loadPersistedSessions();
        }
        
        // 启动定时器
        sessionCheckTimer_->setInterval(config_.sessionCheckIntervalSeconds * 1000);
        sessionCheckTimer_->start();
        
        cleanupTimer_->setInterval(10 * 60 * 1000); // 10分钟清理一次
        cleanupTimer_->start();
        
        isInitialized_ = true;
        qDebug() << "SessionManager initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        emit sessionError("Initialize", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

QString SessionManager::createSession(const QString& username, bool rememberMe, const QString& clientInfo) {
    if (!isInitialized_ || username.isEmpty()) {
        return QString();
    }
    
    try {
        // 检查会话数量限制
        if (!checkSessionLimits(username)) {
            emit sessionError("CreateSession", "Session limit exceeded");
            return QString();
        }
        
        // 创建新会话
        SessionInfo newSession;
        newSession.sessionId = generateSessionId();
        newSession.username = username;
        newSession.createdAt = QDateTime::currentDateTime();
        newSession.lastAccessAt = newSession.createdAt;
        newSession.expiresAt = calculateExpiryTime(rememberMe);
        newSession.clientInfo = clientInfo.isEmpty() ? generateClientInfo() : clientInfo;
        newSession.rememberMe = rememberMe;
        newSession.state = SessionState::Active;
        
        // 如果不允许多会话，销毁该用户的其他会话
        if (!config_.allowMultipleSessions) {
            destroyUserSessions(username);
        }
        
        // 设置为当前会话
        currentSession_ = newSession;
        
        // 持久化存储
        if (config_.persistSessions) {
            saveSessionToPersistentStorage(newSession);
        }
        
        emit sessionCreated(newSession.sessionId, username);
        qDebug() << "Session created for user:" << username << "ID:" << newSession.sessionId;
        
        return newSession.sessionId;
        
    } catch (const std::exception& e) {
        emit sessionError("CreateSession", QString("Exception: %1").arg(e.what()));
        return QString();
    }
}

bool SessionManager::validateSession(const QString& sessionId) {
    if (!isInitialized_ || sessionId.isEmpty()) {
        return false;
    }
    
    // 检查当前会话
    if (currentSession_.sessionId == sessionId) {
        if (currentSession_.isValid()) {
            return true;
        } else if (currentSession_.isExpired()) {
            // 会话过期，清理
            emit sessionExpired(sessionId, currentSession_.username);
            destroySession(sessionId);
            return false;
        }
    }
    
    // 检查持久化存储中的会话
    if (config_.persistSessions && sessionStorage_) {
        SessionInfo sessionInfo = getSessionInfo(sessionId);
        if (sessionInfo.isValid()) {
            currentSession_ = sessionInfo;
            return true;
        }
    }
    
    return false;
}

bool SessionManager::refreshSession(const QString& sessionId) {
    QString targetSessionId = sessionId.isEmpty() ? currentSession_.sessionId : sessionId;
    
    if (targetSessionId.isEmpty()) {
        return false;
    }
    
    if (currentSession_.sessionId == targetSessionId) {
        currentSession_.lastAccessAt = QDateTime::currentDateTime();
        
        // 如果接近过期，延长会话时间
        qint64 remainingTime = currentSession_.getRemainingTime();
        if (remainingTime < 1800) { // 少于30分钟时自动延长
            currentSession_.expiresAt = calculateExpiryTime(currentSession_.rememberMe);
        }
        
        if (config_.persistSessions) {
            saveSessionToPersistentStorage(currentSession_);
        }
        
        emit sessionRefreshed(targetSessionId);
        return true;
    }
    
    return false;
}

bool SessionManager::destroySession(const QString& sessionId) {
    QString targetSessionId = sessionId.isEmpty() ? currentSession_.sessionId : sessionId;
    
    if (targetSessionId.isEmpty()) {
        return false;
    }
    
    try {
        QString username;
        
        // 如果是当前会话
        if (currentSession_.sessionId == targetSessionId) {
            username = currentSession_.username;
            currentSession_ = SessionInfo(); // 重置为无效会话
        }
        
        // 从持久化存储中移除
        if (config_.persistSessions) {
            removeSessionFromPersistentStorage(targetSessionId);
        }
        
        emit sessionDestroyed(targetSessionId, username);
        qDebug() << "Session destroyed:" << targetSessionId;
        
        return true;
        
    } catch (const std::exception& e) {
        emit sessionError("DestroySession", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

bool SessionManager::hasActiveSession() const {
    return currentSession_.isValid();
}

qint64 SessionManager::getSessionRemainingTime(const QString& sessionId) const {
    QString targetSessionId = sessionId.isEmpty() ? currentSession_.sessionId : sessionId;

    if (targetSessionId.isEmpty()) {
        return -1;
    }

    if (currentSession_.sessionId == targetSessionId) {
        return currentSession_.getRemainingTime();
    }

    // 检查持久化存储中的会话
    if (config_.persistSessions && sessionStorage_) {
        SessionInfo sessionInfo = getSessionInfo(targetSessionId);
        return sessionInfo.getRemainingTime();
    }

    return -1;
}

SessionInfo SessionManager::getSessionInfo(const QString& sessionId) const {
    if (sessionId.isEmpty()) {
        return SessionInfo();
    }

    // 检查当前会话
    if (currentSession_.sessionId == sessionId) {
        return currentSession_;
    }

    // 检查持久化存储
    if (config_.persistSessions && sessionStorage_) {
        QString sessionGroup = QString("Sessions/%1").arg(sessionId);
        if (sessionStorage_->contains(sessionGroup + "/sessionId")) {
            sessionStorage_->beginGroup(sessionGroup);
            QString sessionJson = sessionStorage_->value("sessionData").toString();
            sessionStorage_->endGroup();

            if (!sessionJson.isEmpty()) {
                QJsonParseError error;
                QJsonDocument doc = QJsonDocument::fromJson(sessionJson.toUtf8(), &error);
                if (error.error == QJsonParseError::NoError) {
                    return SessionInfo::fromJson(doc.object());
                }
            }
        }
    }

    return SessionInfo();
}

bool SessionManager::extendSession(const QString& sessionId, int minutes) {
    QString targetSessionId = sessionId.isEmpty() ? currentSession_.sessionId : sessionId;

    if (targetSessionId.isEmpty() || minutes <= 0) {
        return false;
    }

    if (currentSession_.sessionId == targetSessionId) {
        currentSession_.expiresAt = currentSession_.expiresAt.addSecs(minutes * 60);

        if (config_.persistSessions) {
            saveSessionToPersistentStorage(currentSession_);
        }

        qDebug() << "Session extended:" << targetSessionId << "by" << minutes << "minutes";
        return true;
    }

    return false;
}

int SessionManager::destroyUserSessions(const QString& username) {
    if (username.isEmpty()) {
        return 0;
    }

    int destroyedCount = 0;

    // 销毁当前会话（如果属于该用户）
    if (currentSession_.username == username) {
        destroySession(currentSession_.sessionId);
        destroyedCount++;
    }

    // 销毁持久化存储中的会话
    if (config_.persistSessions && sessionStorage_) {
        sessionStorage_->beginGroup("Sessions");
        QStringList sessionIds = sessionStorage_->childGroups();
        sessionStorage_->endGroup();

        for (const QString& sessionId : sessionIds) {
            SessionInfo sessionInfo = getSessionInfo(sessionId);
            if (sessionInfo.username == username) {
                removeSessionFromPersistentStorage(sessionId);
                destroyedCount++;
            }
        }
    }

    qDebug() << "Destroyed" << destroyedCount << "sessions for user:" << username;
    return destroyedCount;
}

QStringList SessionManager::getUserActiveSessions(const QString& username) const {
    QStringList activeSessions;

    if (username.isEmpty()) {
        return activeSessions;
    }

    // 检查当前会话
    if (currentSession_.username == username && currentSession_.isValid()) {
        activeSessions.append(currentSession_.sessionId);
    }

    // 检查持久化存储中的会话
    if (config_.persistSessions && sessionStorage_) {
        sessionStorage_->beginGroup("Sessions");
        QStringList sessionIds = sessionStorage_->childGroups();
        sessionStorage_->endGroup();

        for (const QString& sessionId : sessionIds) {
            SessionInfo sessionInfo = getSessionInfo(sessionId);
            if (sessionInfo.username == username && sessionInfo.isValid()) {
                if (!activeSessions.contains(sessionId)) {
                    activeSessions.append(sessionId);
                }
            }
        }
    }

    return activeSessions;
}

int SessionManager::getActiveSessionCount() const {
    int count = 0;

    // 计算当前会话
    if (currentSession_.isValid()) {
        count++;
    }

    // 计算持久化存储中的活跃会话
    if (config_.persistSessions && sessionStorage_) {
        sessionStorage_->beginGroup("Sessions");
        QStringList sessionIds = sessionStorage_->childGroups();
        sessionStorage_->endGroup();

        for (const QString& sessionId : sessionIds) {
            if (sessionId != currentSession_.sessionId) {
                SessionInfo sessionInfo = getSessionInfo(sessionId);
                if (sessionInfo.isValid()) {
                    count++;
                }
            }
        }
    }

    return count;
}

// ==================== 自动登录管理 ====================

QString SessionManager::setAutoLoginToken(const QString& username) {
    if (username.isEmpty()) {
        return QString();
    }

    try {
        QString token = generateSecureToken(username);
        QDateTime expiryTime = QDateTime::currentDateTime().addDays(config_.rememberMeDays);

        if (config_.persistSessions && sessionStorage_) {
            QString tokenGroup = QString("AutoLogin/%1").arg(username);
            sessionStorage_->beginGroup(tokenGroup);
            sessionStorage_->setValue("token", token);
            sessionStorage_->setValue("expiresAt", expiryTime.toString(Qt::ISODate));
            sessionStorage_->setValue("createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));
            sessionStorage_->endGroup();
            sessionStorage_->sync();
        }

        qDebug() << "Auto-login token set for user:" << username;
        return token;

    } catch (const std::exception& e) {
        emit sessionError("SetAutoLoginToken", QString("Exception: %1").arg(e.what()));
        return QString();
    }
}

QString SessionManager::validateAutoLoginToken(const QString& token) {
    if (token.isEmpty() || !config_.persistSessions || !sessionStorage_) {
        return QString();
    }

    try {
        sessionStorage_->beginGroup("AutoLogin");
        QStringList users = sessionStorage_->childGroups();
        sessionStorage_->endGroup();

        for (const QString& username : users) {
            QString tokenGroup = QString("AutoLogin/%1").arg(username);
            sessionStorage_->beginGroup(tokenGroup);

            QString storedToken = sessionStorage_->value("token").toString();
            QString expiryStr = sessionStorage_->value("expiresAt").toString();

            sessionStorage_->endGroup();

            if (storedToken == token) {
                QDateTime expiryTime = QDateTime::fromString(expiryStr, Qt::ISODate);
                if (QDateTime::currentDateTime() < expiryTime) {
                    qDebug() << "Auto-login token validated for user:" << username;
                    return username;
                } else {
                    // 令牌过期，清理
                    clearAutoLoginToken(username);
                }
            }
        }

        return QString();

    } catch (const std::exception& e) {
        emit sessionError("ValidateAutoLoginToken", QString("Exception: %1").arg(e.what()));
        return QString();
    }
}

bool SessionManager::clearAutoLoginToken(const QString& username) {
    if (username.isEmpty() || !config_.persistSessions || !sessionStorage_) {
        return false;
    }

    try {
        QString tokenGroup = QString("AutoLogin/%1").arg(username);
        sessionStorage_->remove(tokenGroup);
        sessionStorage_->sync();

        qDebug() << "Auto-login token cleared for user:" << username;
        return true;

    } catch (const std::exception& e) {
        emit sessionError("ClearAutoLoginToken", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

// ==================== 配置管理 ====================

void SessionManager::setSessionConfig(const SessionConfig& config) {
    config_ = config;

    // 更新定时器间隔
    if (sessionCheckTimer_) {
        sessionCheckTimer_->setInterval(config_.sessionCheckIntervalSeconds * 1000);
    }
}

// ==================== 实用工具 ====================

QString SessionManager::generateSessionId() {
    QUuid uuid = QUuid::createUuid();
    QString sessionId = uuid.toString(QUuid::WithoutBraces);

    // 添加时间戳增强唯一性
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    sessionId += "_" + QString::number(timestamp);

    return sessionId;
}

QString SessionManager::generateSecureToken(const QString& data) {
    QUuid uuid = QUuid::createUuid();
    QString baseToken = uuid.toString(QUuid::WithoutBraces);

    // 添加数据和时间戳
    QString tokenData = baseToken + data + QString::number(QDateTime::currentMSecsSinceEpoch());

    // 使用SHA256哈希
    QByteArray hash = QCryptographicHash::hash(tokenData.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

QString SessionManager::sessionStateToString(SessionState state) {
    switch (state) {
        case SessionState::Invalid: return "无效";
        case SessionState::Active: return "活跃";
        case SessionState::Expired: return "已过期";
        case SessionState::Suspended: return "暂停";
        case SessionState::Terminated: return "已终止";
    }
    return "未知";
}

// ==================== 私有辅助方法 ====================

bool SessionManager::loadPersistedSessions() {
    if (!config_.persistSessions || !sessionStorage_) {
        return true;
    }

    try {
        // 检查是否有当前用户的持久化会话
        sessionStorage_->beginGroup("CurrentSession");
        QString currentSessionJson = sessionStorage_->value("sessionData").toString();
        sessionStorage_->endGroup();

        if (!currentSessionJson.isEmpty()) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(currentSessionJson.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError) {
                SessionInfo sessionInfo = SessionInfo::fromJson(doc.object());
                if (sessionInfo.isValid()) {
                    currentSession_ = sessionInfo;
                    qDebug() << "Loaded persisted session for user:" << sessionInfo.username;
                }
            }
        }

        return true;

    } catch (const std::exception& e) {
        emit sessionError("LoadPersistedSessions", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

bool SessionManager::saveSessionToPersistentStorage(const SessionInfo& sessionInfo) {
    if (!config_.persistSessions || !sessionStorage_) {
        return false;
    }

    try {
        // 保存会话到Sessions组
        QString sessionGroup = QString("Sessions/%1").arg(sessionInfo.sessionId);
        sessionStorage_->beginGroup(sessionGroup);

        QJsonDocument doc(sessionInfo.toJson());
        sessionStorage_->setValue("sessionData", QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
        sessionStorage_->setValue("username", sessionInfo.username);
        sessionStorage_->setValue("createdAt", sessionInfo.createdAt.toString(Qt::ISODate));

        sessionStorage_->endGroup();

        // 如果是当前会话，也保存到CurrentSession组
        if (sessionInfo.sessionId == currentSession_.sessionId) {
            sessionStorage_->beginGroup("CurrentSession");
            sessionStorage_->setValue("sessionData", QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
            sessionStorage_->endGroup();
        }

        sessionStorage_->sync();
        return true;

    } catch (const std::exception& e) {
        emit sessionError("SaveSession", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

bool SessionManager::removeSessionFromPersistentStorage(const QString& sessionId) {
    if (!config_.persistSessions || !sessionStorage_ || sessionId.isEmpty()) {
        return false;
    }

    try {
        // 从Sessions组移除
        QString sessionGroup = QString("Sessions/%1").arg(sessionId);
        sessionStorage_->remove(sessionGroup);

        // 如果是当前会话，也清除CurrentSession
        if (sessionId == currentSession_.sessionId) {
            sessionStorage_->remove("CurrentSession");
        }

        sessionStorage_->sync();
        return true;

    } catch (const std::exception& e) {
        emit sessionError("RemoveSession", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

int SessionManager::cleanupExpiredSessions() {
    int cleanedCount = 0;

    try {
        // 清理当前会话（如果过期）
        if (currentSession_.isExpired()) {
            QString expiredSessionId = currentSession_.sessionId;
            QString expiredUsername = currentSession_.username;
            destroySession(expiredSessionId);
            emit sessionExpired(expiredSessionId, expiredUsername);
            cleanedCount++;
        }

        // 清理持久化存储中的过期会话
        if (config_.persistSessions && sessionStorage_) {
            sessionStorage_->beginGroup("Sessions");
            QStringList sessionIds = sessionStorage_->childGroups();
            sessionStorage_->endGroup();

            for (const QString& sessionId : sessionIds) {
                SessionInfo sessionInfo = getSessionInfo(sessionId);
                if (sessionInfo.isExpired()) {
                    removeSessionFromPersistentStorage(sessionId);
                    emit sessionExpired(sessionId, sessionInfo.username);
                    cleanedCount++;
                }
            }
        }

        if (cleanedCount > 0) {
            qDebug() << "Cleaned up" << cleanedCount << "expired sessions";
        }

        return cleanedCount;

    } catch (const std::exception& e) {
        emit sessionError("CleanupSessions", QString("Exception: %1").arg(e.what()));
        return 0;
    }
}

bool SessionManager::checkSessionLimits(const QString& username) {
    if (!config_.allowMultipleSessions) {
        return true; // 如果不允许多会话，总是可以创建（会替换现有会话）
    }

    QStringList activeSessions = getUserActiveSessions(username);
    return activeSessions.size() < config_.maxConcurrentSessions;
}

QDateTime SessionManager::calculateExpiryTime(bool rememberMe) const {
    QDateTime currentTime = QDateTime::currentDateTime();

    if (rememberMe) {
        return currentTime.addDays(config_.rememberMeDays);
    } else {
        return currentTime.addSecs(config_.defaultTimeoutMinutes * 60);
    }
}

QString SessionManager::generateClientInfo() const {
    // 生成基本的客户端信息
    QString clientInfo = QString("UnravelChat_%1_%2")
                        .arg(QDateTime::currentDateTime().toString("yyyyMMdd"))
                        .arg(QDateTime::currentMSecsSinceEpoch());
    return clientInfo;
}

// ==================== 定时器槽函数 ====================

void SessionManager::onSessionCheck() {
    // 检查当前会话状态
    if (currentSession_.isValid()) {
        qint64 remainingTime = currentSession_.getRemainingTime();

        // 如果会话即将过期（少于10分钟），发出警告
        if (remainingTime > 0 && remainingTime < 600) {
            qDebug() << "Session will expire in" << remainingTime << "seconds for user:" << currentSession_.username;
        }

        // 如果已过期，销毁会话
        if (remainingTime <= 0) {
            QString expiredSessionId = currentSession_.sessionId;
            QString expiredUsername = currentSession_.username;
            destroySession(expiredSessionId);
            emit sessionExpired(expiredSessionId, expiredUsername);
        }
    }
}

void SessionManager::onCleanupTimer() {
    cleanupExpiredSessions();
}

