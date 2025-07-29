#include "../include/auth/AuthenticationManager.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <iostream>

class AuthTester : public QObject {
    Q_OBJECT

private:
    AuthenticationManager* authManager_;

public:
    explicit AuthTester(QObject* parent = nullptr) : QObject(parent) {
        authManager_ = new AuthenticationManager(this);
        
        // 连接信号
        connect(authManager_, &AuthenticationManager::userRegistered,
                this, [](const QString& username) {
                    std::cout << "✓ 用户注册成功: " << username.toStdString() << std::endl;
                });
                
        connect(authManager_, &AuthenticationManager::userLoggedIn,
                this, [](const QString& username) {
                    std::cout << "✓ 用户登录成功: " << username.toStdString() << std::endl;
                });
                
        connect(authManager_, &AuthenticationManager::userLoggedOut,
                this, [](const QString& username) {
                    std::cout << "✓ 用户登出: " << username.toStdString() << std::endl;
                });
                
        connect(authManager_, &AuthenticationManager::accountLocked,
                this, [](const QString& username, int duration) {
                    std::cout << "⚠️ 账户被锁定: " << username.toStdString() 
                              << " 时长: " << duration << "分钟" << std::endl;
                });
                
        connect(authManager_, &AuthenticationManager::authenticationError,
                this, [](AuthenticationResult, const QString& message) {
                    std::cout << "❌ 认证错误: " << message.toStdString() << std::endl;
                });
    }

    void runTest() {
        std::cout << "========================================" << std::endl;
        std::cout << "开始AuthenticationManager功能测试..." << std::endl;
        std::cout << "========================================" << std::endl;
        
        testInitialization();
        testUserRegistration();
        testUserLogin();
        testSecurityFeatures();
        testSessionManagement();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "AuthenticationManager功能测试完成！" << std::endl;
        std::cout << "========================================" << std::endl;
        
        QTimer::singleShot(2000, qApp, &QCoreApplication::quit);
    }

private:
    void testInitialization() {
        std::cout << "\n[测试1] 认证管理器初始化..." << std::endl;
        
        bool initResult = authManager_->initialize();
        std::cout << "初始化结果: " << (initResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        if (initResult) {
            std::cout << "用户总数: " << authManager_->getUserCount() << std::endl;
        }
    }
    
    void testUserRegistration() {
        std::cout << "\n[测试2] 用户注册测试..." << std::endl;
        
        // 测试正常注册
        AuthenticationResult result = authManager_->registerUser(
            "testuser001", "StrongPass123!", "test@example.com", "测试用户");
        std::cout << "正常注册: " << (result == AuthenticationResult::Success ? "✓ 成功" : "✗ 失败") 
                  << " - " << AuthenticationManager::getResultDescription(result).toStdString() << std::endl;
        
        // 测试重复用户名
        result = authManager_->registerUser(
            "testuser001", "AnotherPass123!", "test2@example.com", "重复用户");
        std::cout << "重复用户名(应该失败): " << (result == AuthenticationResult::UserAlreadyExists ? "✓ 成功" : "✗ 失败")
                  << " - " << AuthenticationManager::getResultDescription(result).toStdString() << std::endl;
        
        // 测试弱密码
        result = authManager_->registerUser(
            "weakuser", "123", "weak@example.com", "弱密码用户");
        std::cout << "弱密码(应该失败): " << (result == AuthenticationResult::WeakPassword ? "✓ 成功" : "✗ 失败")
                  << " - " << AuthenticationManager::getResultDescription(result).toStdString() << std::endl;
        
        // 测试无效邮箱
        result = authManager_->registerUser(
            "invaliduser", "ValidPass123!", "invalid-email", "无效邮箱用户");
        std::cout << "无效邮箱(应该失败): " << (result == AuthenticationResult::InvalidEmail ? "✓ 成功" : "✗ 失败")
                  << " - " << AuthenticationManager::getResultDescription(result).toStdString() << std::endl;
        
        std::cout << "注册后用户总数: " << authManager_->getUserCount() << std::endl;
    }
    
    void testUserLogin() {
        std::cout << "\n[测试3] 用户登录测试..." << std::endl;
        
        // 测试正确登录
        AuthenticationResult result = authManager_->loginUser("testuser001", "StrongPass123!");
        std::cout << "正确登录: " << (result == AuthenticationResult::Success ? "✓ 成功" : "✗ 失败")
                  << " - " << AuthenticationManager::getResultDescription(result).toStdString() << std::endl;
        
        if (result == AuthenticationResult::Success) {
            std::cout << "当前登录用户: " << authManager_->getCurrentUsername().toStdString() << std::endl;
            std::cout << "是否已登录: " << (authManager_->isUserLoggedIn() ? "是" : "否") << std::endl;
            
            UserInfo currentUser = authManager_->getCurrentUserInfo();
            if (currentUser.isValid()) {
                std::cout << "用户邮箱: " << currentUser.email.toStdString() << std::endl;
                std::cout << "显示名称: " << currentUser.displayName.toStdString() << std::endl;
            }
        }
        
        // 测试邮箱登录
        authManager_->logoutUser();
        result = authManager_->loginUser("test@example.com", "StrongPass123!");
        std::cout << "邮箱登录: " << (result == AuthenticationResult::Success ? "✓ 成功" : "✗ 失败")
                  << " - " << AuthenticationManager::getResultDescription(result).toStdString() << std::endl;
        
        // 测试错误密码
        authManager_->logoutUser();
        result = authManager_->loginUser("testuser001", "WrongPassword");
        std::cout << "错误密码(应该失败): " << (result == AuthenticationResult::InvalidCredentials ? "✓ 成功" : "✗ 失败")
                  << " - " << AuthenticationManager::getResultDescription(result).toStdString() << std::endl;
        
        // 测试不存在的用户
        result = authManager_->loginUser("nonexistent", "AnyPassword");
        std::cout << "不存在用户(应该失败): " << (result == AuthenticationResult::UserNotFound ? "✓ 成功" : "✗ 失败")
                  << " - " << AuthenticationManager::getResultDescription(result).toStdString() << std::endl;
    }
    
    void testSecurityFeatures() {
        std::cout << "\n[测试4] 安全特性测试..." << std::endl;
        
        // 测试多次失败登录
        std::cout << "测试暴力破解保护..." << std::endl;
        for (int i = 1; i <= 6; ++i) {
            AuthenticationResult result = authManager_->loginUser("testuser001", "WrongPassword");
            int attemptCount = authManager_->getLoginAttemptCount("testuser001");
            std::cout << "第" << i << "次失败登录, 尝试次数: " << attemptCount << std::endl;
            
            if (result == AuthenticationResult::AccountLocked) {
                std::cout << "账户已被锁定" << std::endl;
                break;
            }
        }
        
        // 检查锁定状态
        bool isLocked = authManager_->isAccountLocked("testuser001");
        std::cout << "账户锁定状态: " << (isLocked ? "已锁定" : "未锁定") << std::endl;
        
        if (isLocked) {
            int remainingTime = authManager_->getRemainingLockoutTime("testuser001");
            std::cout << "剩余锁定时间: " << remainingTime << " 秒" << std::endl;
            
            // 测试管理员解锁
            bool unlockResult = authManager_->unlockAccount("testuser001");
            std::cout << "管理员解锁: " << (unlockResult ? "✓ 成功" : "✗ 失败") << std::endl;
            
            isLocked = authManager_->isAccountLocked("testuser001");
            std::cout << "解锁后状态: " << (isLocked ? "仍锁定" : "已解锁") << std::endl;
        }
    }
    
    void testSessionManagement() {
        std::cout << "\n[测试5] 会话管理测试..." << std::endl;
        
        // 重新登录
        AuthenticationResult result = authManager_->loginUser("testuser001", "StrongPass123!");
        if (result == AuthenticationResult::Success) {
            std::cout << "登录成功，开始会话测试" << std::endl;
            
            // 测试会话信息
            int sessionDuration = authManager_->getSessionDuration();
            std::cout << "会话持续时间: " << sessionDuration << " 秒" << std::endl;
            
            // 刷新活动时间
            authManager_->refreshUserActivity();
            std::cout << "用户活动时间已刷新" << std::endl;
            
            // 获取登录历史
            auto loginHistory = authManager_->getLoginHistory("testuser001", 5);
            std::cout << "最近5次登录记录:" << std::endl;
            for (const auto& attempt : loginHistory) {
                std::cout << "  " << attempt.timestamp.toString().toStdString()
                          << " - " << (attempt.successful ? "成功" : "失败")
                          << " - " << attempt.failureReason.toStdString() << std::endl;
            }
            
            // 测试登出
            authManager_->logoutUser();
            std::cout << "用户已登出" << std::endl;
            std::cout << "登出后登录状态: " << (authManager_->isUserLoggedIn() ? "仍登录" : "已登出") << std::endl;
        }
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("UnravelChat");
    app.setOrganizationName("unravel");
    
    AuthTester tester;
    QTimer::singleShot(100, &tester, &AuthTester::runTest);
    
    return app.exec();
}

#include "test_authmanager.moc"

