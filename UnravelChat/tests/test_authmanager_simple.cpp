#include "../include/auth/AuthenticationManager.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <iostream>

class SimpleAuthTester : public QObject {
    Q_OBJECT

private:
    AuthenticationManager* authManager_;

public:
    explicit SimpleAuthTester(QObject* parent = nullptr) : QObject(parent) {
        authManager_ = new AuthenticationManager(this);
    }

    void runTest() {
        std::cout << "========================================" << std::endl;
        std::cout << "简化版AuthenticationManager测试..." << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 测试1: 初始化
        std::cout << "\n[测试1] 初始化..." << std::endl;
        bool initResult = authManager_->initialize();
        std::cout << "初始化: " << (initResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        if (!initResult) {
            QTimer::singleShot(100, qApp, &QCoreApplication::quit);
            return;
        }
        
        // 测试2: 检查可用性
        std::cout << "\n[测试2] 检查可用性..." << std::endl;
        bool usernameAvailable = authManager_->isUsernameAvailable("newuser123");
        bool emailAvailable = authManager_->isEmailAvailable("new@test.com");
        std::cout << "用户名可用: " << (usernameAvailable ? "是" : "否") << std::endl;
        std::cout << "邮箱可用: " << (emailAvailable ? "是" : "否") << std::endl;
        
        // 测试3: 简单注册
        std::cout << "\n[测试3] 用户注册..." << std::endl;
        AuthenticationResult regResult = authManager_->registerUser(
            "simpleuser", "SimplePass123!", "simple@test.com", "简单用户");
        
        std::cout << "注册结果: " << AuthenticationManager::getResultDescription(regResult).toStdString() << std::endl;
        
        if (regResult == AuthenticationResult::Success) {
            std::cout << "✓ 注册成功" << std::endl;
            
            // 测试4: 登录
            std::cout << "\n[测试4] 用户登录..." << std::endl;
            AuthenticationResult loginResult = authManager_->loginUser("simpleuser", "SimplePass123!");
            std::cout << "登录结果: " << AuthenticationManager::getResultDescription(loginResult).toStdString() << std::endl;
            
            if (loginResult == AuthenticationResult::Success) {
                std::cout << "✓ 登录成功" << std::endl;
                std::cout << "当前用户: " << authManager_->getCurrentUsername().toStdString() << std::endl;
                
                // 登出
                authManager_->logoutUser();
                std::cout << "✓ 已登出" << std::endl;
            }
        } else {
            std::cout << "✗ 注册失败: " << AuthenticationManager::getResultDescription(regResult).toStdString() << std::endl;
        }
        
        std::cout << "\n用户总数: " << authManager_->getUserCount() << std::endl;
        std::cout << "测试完成！" << std::endl;
        
        QTimer::singleShot(1000, qApp, &QCoreApplication::quit);
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("UnravelChat");
    app.setOrganizationName("unravel");
    
    SimpleAuthTester tester;
    QTimer::singleShot(100, &tester, &SimpleAuthTester::runTest);
    
    return app.exec();
}

#include "test_authmanager_simple.moc"

