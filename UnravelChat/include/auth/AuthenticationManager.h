/**
 * @file AuthenticationManager.h
 * @author unravel
 * @date 2025-07-28
 * @framework Qt 6.x, C++17 Standard
 * @technology 集成认证系统, 业务逻辑层, 安全控制
 * @task UnravelChat第二阶段 - 认证管理器
 */

#ifndef AUTHENTICATIONMANAGER_H
#define AUTHENTICATIONMANAGER_H

#include "UserDatabase.h"
#include "PasswordManager.h"
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QRegularExpression>
#include <unordered_map>
#include <memory>

enum class AuthenticationResult {
    Success,
    InvalidCredentials,
    UserNotFound,
    UserAlreadyExists,
    InvalidEmail,
    InvalidUsername,
    WeakPassword,
    AccountLocked,
    TooManyAttempts,
    DatabaseError,
    UnknownError
};

struct LoginAttempt {
    QString username;
    QDateTime timestamp;
    bool successful;
    QString failureReason;
    
    LoginAttempt() : successful(false) {}
    LoginAttempt(const QString& user, bool success, const QString& reason = "")
        : username(user), timestamp(QDateTime::currentDateTime()), 
          successful(success), failureReason(reason) {}
};

struct SecurityConfig {
    int maxLoginAttempts = 5;
    int lockoutDurationMinutes = 15;
    int sessionTimeoutMinutes = 120;
    bool enableBruteForceProtection = true;
    bool logSecurityEvents = true;
    
    static SecurityConfig defaultConfig() {
        return SecurityConfig{};
    }
};

class AuthenticationManager : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<UserDatabase> userDatabase_;
    std::unique_ptr<PasswordManager> passwordManager_;
    
    SecurityConfig securityConfig_;
    std::unordered_map<std::string, int> loginAttempts_;
    std::unordered_map<std::string, QDateTime> lockoutTime_;
    std::vector<LoginAttempt> loginHistory_;
    
    QString currentUser_;
    QDateTime loginTime_;
    QDateTime lastActivity_;
    bool isLoggedIn_;
    
    mutable QMutex mutex_;
    QTimer* cleanupTimer_;
    
    static constexpr int MAX_LOGIN_HISTORY = 1000;

public:
    explicit AuthenticationManager(QObject* parent = nullptr, 
                                 const SecurityConfig& config = SecurityConfig::defaultConfig());
    ~AuthenticationManager();
    
    bool initialize();

    // 用户注册
    AuthenticationResult registerUser(const QString& username, const QString& password,
                                    const QString& email, const QString& displayName = "");
    bool isUsernameAvailable(const QString& username) const;
    bool isEmailAvailable(const QString& email) const;
    
    // 用户登录
    AuthenticationResult loginUser(const QString& username, const QString& password);
    void logoutUser();
    bool isUserLoggedIn() const { return isLoggedIn_; }
    QString getCurrentUsername() const { return currentUser_; }
    UserInfo getCurrentUserInfo() const;
    void refreshUserActivity();
    
    // 密码管理
    AuthenticationResult changePassword(const QString& currentPassword, const QString& newPassword);
    
    // 安全控制
    bool isAccountLocked(const QString& username) const;
    int getRemainingLockoutTime(const QString& username) const;
    bool unlockAccount(const QString& username);
    int getLoginAttemptCount(const QString& username) const;
    
    // 配置管理
    void setSecurityConfig(const SecurityConfig& config);
    const SecurityConfig& getSecurityConfig() const { return securityConfig_; }
    
    // 用户管理
    QStringList getAllUsers() const;
    int getUserCount() const;
    bool deleteUser(const QString& username);
    
    // 审计监控
    std::vector<LoginAttempt> getLoginHistory(const QString& username = "", int limit = 50) const;
    int getSessionDuration() const;
    
    // 实用工具
    static QString getResultDescription(AuthenticationResult result);
    static bool isValidEmail(const QString& email);
    static bool isValidUsername(const QString& username);

private:
    void recordLoginAttempt(const QString& username, bool successful, const QString& reason = "");
    void incrementLoginFailures(const QString& username);
    void clearLoginFailures(const QString& username);
    void lockAccount(const QString& username);
    QString resolveUsername(const QString& usernameOrEmail) const;
    bool isSessionValid() const;
    void performCleanup();

private slots:
    void onCleanupTimer();

signals:
    void userRegistered(const QString& username);
    void userLoggedIn(const QString& username);
    void userLoggedOut(const QString& username);
    void accountLocked(const QString& username, int duration);
    void passwordChanged(const QString& username);
    void sessionExpired(const QString& username);
    void authenticationError(AuthenticationResult result, const QString& message);
};

#endif // AUTHENTICATIONMANAGER_H

