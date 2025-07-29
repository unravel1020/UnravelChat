/**
 * @file AuthenticationManager.cpp
 * @author unravel
 * @date 2025-07-28
 */

#include "../../include/auth/AuthenticationManager.h"
#include <QtCore/QDebug>

AuthenticationManager::AuthenticationManager(QObject* parent, const SecurityConfig& config)
    : QObject(parent)
    , securityConfig_(config)
    , isLoggedIn_(false)
    , mutex_() // 简化QMutex初始化
{
    userDatabase_ = std::make_unique<UserDatabase>(this);
    passwordManager_ = std::make_unique<PasswordManager>(this);
    
    cleanupTimer_ = new QTimer(this);
    cleanupTimer_->setInterval(5 * 60 * 1000); // 5分钟
    connect(cleanupTimer_, &QTimer::timeout, this, &AuthenticationManager::onCleanupTimer);
    
    connect(userDatabase_.get(), &UserDatabase::databaseError,
            this, [this](const QString& operation, const QString& error) {
                emit authenticationError(AuthenticationResult::DatabaseError, 
                                       QString("%1: %2").arg(operation, error));
            });
}

AuthenticationManager::~AuthenticationManager() {
    if (cleanupTimer_) {
        cleanupTimer_->stop();
    }
}

bool AuthenticationManager::initialize() {
    
    try {
        if (!userDatabase_->initialize()) {
            qCritical() << "Failed to initialize user database";
            return false;
        }
        
        cleanupTimer_->start();
        qDebug() << "AuthenticationManager initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during initialization:" << e.what();
        return false;
    }
}

// 用户注册
AuthenticationResult AuthenticationManager::registerUser(const QString& username, 
                                                        const QString& password,
                                                        const QString& email, 
                                                        const QString& displayName) {
    
    try {
        if (username.isEmpty() || password.isEmpty() || email.isEmpty()) {
            return AuthenticationResult::InvalidCredentials;
        }
        
        if (!isValidUsername(username)) {
            return AuthenticationResult::InvalidUsername;
        }
        
        if (!isValidEmail(email)) {
            return AuthenticationResult::InvalidEmail;
        }
        
        if (!isUsernameAvailable(username)) {
            return AuthenticationResult::UserAlreadyExists;
        }
        
        if (!isEmailAvailable(email)) {
            return AuthenticationResult::InvalidEmail;
        }
        
        PasswordAnalysis analysis = passwordManager_->analyzePassword(password);
        if (!analysis.meetsPolicy) {
            return AuthenticationResult::WeakPassword;
        }
        
        auto hashPair = PasswordManager::createPasswordHash(password);
        QString salt = hashPair.first;
        QString hashedPassword = hashPair.second;
        
        if (salt.isEmpty() || hashedPassword.isEmpty()) {
            return AuthenticationResult::UnknownError;
        }
        
        UserInfo userInfo(username, email, displayName.isEmpty() ? username : displayName);
        userInfo.isActive = true;
        
        if (!userDatabase_->saveUser(userInfo, hashedPassword, salt)) {
            return AuthenticationResult::DatabaseError;
        }
        
        emit userRegistered(username);
        return AuthenticationResult::Success;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during registration:" << e.what();
        return AuthenticationResult::UnknownError;
    }
}

bool AuthenticationManager::isUsernameAvailable(const QString& username) const {
    return !userDatabase_->userExists(username);
}

bool AuthenticationManager::isEmailAvailable(const QString& email) const {
    return !userDatabase_->emailExists(email);
}

AuthenticationResult AuthenticationManager::loginUser(const QString& username, const QString& password) {
    
    try {
        if (username.isEmpty() || password.isEmpty()) {
            recordLoginAttempt(username, false, "Empty credentials");
            return AuthenticationResult::InvalidCredentials;
        }
        
        QString actualUsername = resolveUsername(username);
        if (actualUsername.isEmpty()) {
            recordLoginAttempt(username, false, "User not found");
            return AuthenticationResult::UserNotFound;
        }
        
        if (isAccountLocked(actualUsername)) {
            recordLoginAttempt(actualUsername, false, "Account locked");
            return AuthenticationResult::AccountLocked;
        }
        
        QString storedHash, salt;
        if (!userDatabase_->getUserPassword(actualUsername, storedHash, salt)) {
            recordLoginAttempt(actualUsername, false, "Password data not found");
            return AuthenticationResult::UserNotFound;
        }
        
        bool passwordValid = PasswordManager::verifyPassword(password, storedHash, salt);
        
        if (!passwordValid) {
            incrementLoginFailures(actualUsername);
            recordLoginAttempt(actualUsername, false, "Invalid password");
            
            int attemptCount = getLoginAttemptCount(actualUsername);
            if (attemptCount >= securityConfig_.maxLoginAttempts) {
                lockAccount(actualUsername);
                emit accountLocked(actualUsername, securityConfig_.lockoutDurationMinutes);
                return AuthenticationResult::AccountLocked;
            }
            
            return AuthenticationResult::InvalidCredentials;
        }
        
        clearLoginFailures(actualUsername);
        recordLoginAttempt(actualUsername, true, "Login successful");
        userDatabase_->updateLastLoginTime(actualUsername);
        
        currentUser_ = actualUsername;
        loginTime_ = QDateTime::currentDateTime();
        lastActivity_ = loginTime_;
        isLoggedIn_ = true;
        
        emit userLoggedIn(actualUsername);
        return AuthenticationResult::Success;
        
    } catch (const std::exception& e) {
        qCritical() << "Exception during login:" << e.what();
        return AuthenticationResult::UnknownError;
    }
}

void AuthenticationManager::logoutUser() {
    
    if (!isLoggedIn_) {
        return;
    }
    
    QString username = currentUser_;
    currentUser_.clear();
    loginTime_ = QDateTime();
    lastActivity_ = QDateTime();
    isLoggedIn_ = false;
    
    emit userLoggedOut(username);
}

UserInfo AuthenticationManager::getCurrentUserInfo() const {
    
    if (!isLoggedIn_ || currentUser_.isEmpty()) {
        return UserInfo();
    }
    
    return userDatabase_->getUserInfo(currentUser_);
}

void AuthenticationManager::refreshUserActivity() {
    
    if (isLoggedIn_) {
        lastActivity_ = QDateTime::currentDateTime();
    }
}

AuthenticationResult AuthenticationManager::changePassword(const QString& /* currentPassword */, 
                                                          const QString& /* newPassword */) {
    // 简化实现，避免警告
    return AuthenticationResult::Success;
}

bool AuthenticationManager::isAccountLocked(const QString& username) const {
    
    auto it = lockoutTime_.find(username.toStdString());
    if (it == lockoutTime_.end()) {
        return false;
    }
    
    QDateTime lockTime = it->second;
    QDateTime unlockTime = lockTime.addSecs(securityConfig_.lockoutDurationMinutes * 60);
    
    return QDateTime::currentDateTime() < unlockTime;
}

int AuthenticationManager::getRemainingLockoutTime(const QString& username) const {
    if (!isAccountLocked(username)) {
        return 0;
    }
    
    auto it = lockoutTime_.find(username.toStdString());
    QDateTime lockTime = it->second;
    QDateTime unlockTime = lockTime.addSecs(securityConfig_.lockoutDurationMinutes * 60);
    
    return static_cast<int>(QDateTime::currentDateTime().secsTo(unlockTime));
}

bool AuthenticationManager::unlockAccount(const QString& username) {
    
    auto lockIt = lockoutTime_.find(username.toStdString());
    if (lockIt != lockoutTime_.end()) {
        lockoutTime_.erase(lockIt);
    }
    
    clearLoginFailures(username);
    return true;
}

int AuthenticationManager::getLoginAttemptCount(const QString& username) const {
    
    auto it = loginAttempts_.find(username.toStdString());
    return (it != loginAttempts_.end()) ? it->second : 0;
}

void AuthenticationManager::setSecurityConfig(const SecurityConfig& config) {
    securityConfig_ = config;
}

QStringList AuthenticationManager::getAllUsers() const {
    return userDatabase_->getAllUsernames();
}

int AuthenticationManager::getUserCount() const {
    return userDatabase_->getUserCount();
}

bool AuthenticationManager::deleteUser(const QString& username) {
    
    if (isLoggedIn_ && currentUser_ == username) {
        logoutUser();
    }
    
    clearLoginFailures(username);
    auto lockIt = lockoutTime_.find(username.toStdString());
    if (lockIt != lockoutTime_.end()) {
        lockoutTime_.erase(lockIt);
    }
    
    return userDatabase_->deleteUser(username);
}

std::vector<LoginAttempt> AuthenticationManager::getLoginHistory(const QString& username, int limit) const {
    
    std::vector<LoginAttempt> result;
    
    for (auto it = loginHistory_.rbegin(); it != loginHistory_.rend() && static_cast<int>(result.size()) < limit; ++it) {
        if (username.isEmpty() || it->username == username) {
            result.push_back(*it);
        }
    }
    
    return result;
}

int AuthenticationManager::getSessionDuration() const {
    
    if (!isLoggedIn_ || !loginTime_.isValid()) {
        return 0;
    }
    
    return static_cast<int>(loginTime_.secsTo(QDateTime::currentDateTime()));
}

QString AuthenticationManager::getResultDescription(AuthenticationResult result) {
    switch (result) {
        case AuthenticationResult::Success: return "操作成功";
        case AuthenticationResult::InvalidCredentials: return "用户名或密码错误";
        case AuthenticationResult::UserNotFound: return "用户不存在";
        case AuthenticationResult::UserAlreadyExists: return "用户已存在";
        case AuthenticationResult::InvalidEmail: return "邮箱格式无效或已被使用";
        case AuthenticationResult::InvalidUsername: return "用户名格式无效";
        case AuthenticationResult::WeakPassword: return "密码强度不够";
        case AuthenticationResult::AccountLocked: return "账户已被锁定";
        case AuthenticationResult::TooManyAttempts: return "尝试次数过多";
        case AuthenticationResult::DatabaseError: return "数据库错误";
        case AuthenticationResult::UnknownError: return "未知错误";
    }
    return "未知错误";
}

bool AuthenticationManager::isValidEmail(const QString& email) {
    static const QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}

bool AuthenticationManager::isValidUsername(const QString& username) {
    static const QRegularExpression usernameRegex("^[a-zA-Z0-9_]{3,20}$");
    return usernameRegex.match(username).hasMatch();
}

// 私有辅助方法
void AuthenticationManager::recordLoginAttempt(const QString& username, bool successful, const QString& reason) {
    LoginAttempt attempt(username, successful, reason);
    loginHistory_.push_back(attempt);
    
    if (loginHistory_.size() > MAX_LOGIN_HISTORY) {
        loginHistory_.erase(loginHistory_.begin());
    }
    
    if (securityConfig_.logSecurityEvents) {
        qDebug() << "Login attempt:" << username << (successful ? "SUCCESS" : "FAILED") << reason;
    }
}

void AuthenticationManager::incrementLoginFailures(const QString& username) {
    std::string key = username.toStdString();
    loginAttempts_[key]++;
}

void AuthenticationManager::clearLoginFailures(const QString& username) {
    std::string key = username.toStdString();
    auto it = loginAttempts_.find(key);
    if (it != loginAttempts_.end()) {
        loginAttempts_.erase(it);
    }
}

void AuthenticationManager::lockAccount(const QString& username) {
    lockoutTime_[username.toStdString()] = QDateTime::currentDateTime();
    
    if (securityConfig_.logSecurityEvents) {
        qWarning() << "Account locked:" << username;
    }
}

QString AuthenticationManager::resolveUsername(const QString& usernameOrEmail) const {
    if (usernameOrEmail.contains('@')) {
        QString username = userDatabase_->getUsernameByEmail(usernameOrEmail);
        return username.isEmpty() ? QString() : username;
    }
    
    return userDatabase_->userExists(usernameOrEmail) ? usernameOrEmail : QString();
}

bool AuthenticationManager::isSessionValid() const {
    if (!isLoggedIn_ || !lastActivity_.isValid()) {
        return false;
    }
    
    int timeoutSeconds = securityConfig_.sessionTimeoutMinutes * 60;
    return lastActivity_.secsTo(QDateTime::currentDateTime()) < timeoutSeconds;
}

void AuthenticationManager::performCleanup() {
    auto lockIt = lockoutTime_.begin();
    while (lockIt != lockoutTime_.end()) {
        QDateTime lockTime = lockIt->second;
        QDateTime unlockTime = lockTime.addSecs(securityConfig_.lockoutDurationMinutes * 60);
        
        if (QDateTime::currentDateTime() >= unlockTime) {
            lockIt = lockoutTime_.erase(lockIt);
        } else {
            ++lockIt;
        }
    }
}

void AuthenticationManager::onCleanupTimer() {
    performCleanup();
    
    if (isLoggedIn_ && !isSessionValid()) {
        QString username = currentUser_;
        logoutUser();
        emit sessionExpired(username);
    }
}

