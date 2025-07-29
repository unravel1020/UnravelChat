#include "../include/auth/SessionManager.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <iostream>

class SessionTester : public QObject {
    Q_OBJECT

private:
    SessionManager* sessionManager_;

public:
    explicit SessionTester(QObject* parent = nullptr) : QObject(parent) {
        sessionManager_ = new SessionManager(this);
        
        // 连接信号
        connect(sessionManager_, &SessionManager::sessionCreated,
                this, [](const QString& sessionId, const QString& username) {
                    std::cout << "✓ 会话创建: " << username.toStdString() 
                              << " ID: " << sessionId.left(8).toStdString() << "..." << std::endl;
                });
                
        connect(sessionManager_, &SessionManager::sessionDestroyed,
                this, [](const QString& sessionId, const QString& username) {
                    std::cout << "✓ 会话销毁: " << username.toStdString() 
                              << " ID: " << sessionId.left(8).toStdString() << "..." << std::endl;
                });
                
        connect(sessionManager_, &SessionManager::sessionExpired,
                this, [](const QString& sessionId, const QString& username) {
                    std::cout << "⏰ 会话过期: " << username.toStdString() 
                              << " ID: " << sessionId.left(8).toStdString() << "..." << std::endl;
                });
                
        connect(sessionManager_, &SessionManager::sessionRefreshed,
                this, [](const QString& sessionId) {
                    std::cout << "🔄 会话刷新: " << sessionId.left(8).toStdString() << "..." << std::endl;
                });
                
        connect(sessionManager_, &SessionManager::sessionError,
                this, [](const QString& operation, const QString& error) {
                    std::cout << "❌ 会话错误 [" << operation.toStdString() 
                              << "]: " << error.toStdString() << std::endl;
                });
    }

    void runTest() {
        std::cout << "========================================" << std::endl;
        std::cout << "开始SessionManager功能测试..." << std::endl;
        std::cout << "========================================" << std::endl;
        
        testInitialization();
        testSessionCreation();
        testSessionValidation();
        testSessionManagement();
        testAutoLogin();
        testSessionLimits();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "SessionManager功能测试完成！" << std::endl;
        std::cout << "========================================" << std::endl;
        
        QTimer::singleShot(2000, qApp, &QCoreApplication::quit);
    }

private:
    void testInitialization() {
        std::cout << "\n[测试1] 会话管理器初始化..." << std::endl;
        
        bool initResult = sessionManager_->initialize();
        std::cout << "初始化结果: " << (initResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        if (initResult) {
            int activeCount = sessionManager_->getActiveSessionCount();
            std::cout << "当前活跃会话数: " << activeCount << std::endl;
        }
    }
    
    void testSessionCreation() {
        std::cout << "\n[测试2] 会话创建测试..." << std::endl;
        
        // 创建普通会话
        QString sessionId1 = sessionManager_->createSession("testuser001", false, "TestClient");
        std::cout << "普通会话创建: " << (!sessionId1.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
        if (!sessionId1.isEmpty()) {
            std::cout << "会话ID: " << sessionId1.left(16).toStdString() << "..." << std::endl;
        }
        
        // 检查会话信息
        if (sessionManager_->hasActiveSession()) {
            SessionInfo currentSession = sessionManager_->getCurrentSession();
            std::cout << "当前用户: " << currentSession.username.toStdString() << std::endl;
            std::cout << "创建时间: " << currentSession.createdAt.toString().toStdString() << std::endl;
            std::cout << "过期时间: " << currentSession.expiresAt.toString().toStdString() << std::endl;
            std::cout << "记住登录: " << (currentSession.rememberMe ? "是" : "否") << std::endl;
            
            qint64 remainingTime = sessionManager_->getSessionRemainingTime();
            std::cout << "剩余时间: " << remainingTime << " 秒" << std::endl;
        }
        
        // 创建"记住我"会话
        sessionManager_->destroySession(); // 先销毁当前会话
        QString sessionId2 = sessionManager_->createSession("testuser002", true, "RememberClient");
        std::cout << "记住我会话创建: " << (!sessionId2.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
    }
    
    void testSessionValidation() {
        std::cout << "\n[测试3] 会话验证测试..." << std::endl;
        
        QString currentSessionId = sessionManager_->getCurrentSessionId();
        
        // 验证有效会话
        bool validResult = sessionManager_->validateSession(currentSessionId);
        std::cout << "有效会话验证: " << (validResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 验证无效会话
        bool invalidResult = sessionManager_->validateSession("invalid-session-id");
        std::cout << "无效会话验证(应该失败): " << (!invalidResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 验证空会话ID
        bool emptyResult = sessionManager_->validateSession("");
        std::cout << "空会话ID验证(应该失败): " << (!emptyResult ? "✓ 成功" : "✗ 失败") << std::endl;
    }
    
    void testSessionManagement() {
        std::cout << "\n[测试4] 会话管理测试..." << std::endl;
        
        QString currentSessionId = sessionManager_->getCurrentSessionId();
        
        // 测试会话刷新
        bool refreshResult = sessionManager_->refreshSession();
        std::cout << "会话刷新: " << (refreshResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 测试会话延长
        bool extendResult = sessionManager_->extendSession("", 30); // 延长30分钟
        std::cout << "会话延长: " << (extendResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 获取用户活跃会话
        QString currentUser = sessionManager_->getCurrentUsername();
        QStringList userSessions = sessionManager_->getUserActiveSessions(currentUser);
        std::cout << "用户活跃会话数: " << userSessions.size() << std::endl;
        
        // 测试会话状态
        SessionInfo sessionInfo = sessionManager_->getCurrentSession();
        std::cout << "会话状态: " << SessionManager::sessionStateToString(sessionInfo.state).toStdString() << std::endl;
    }
    
    void testAutoLogin() {
        std::cout << "\n[测试5] 自动登录测试..." << std::endl;
        
        QString currentUser = sessionManager_->getCurrentUsername();
        
        // 设置自动登录令牌
        QString autoLoginToken = sessionManager_->setAutoLoginToken(currentUser);
        std::cout << "自动登录令牌设置: " << (!autoLoginToken.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
        if (!autoLoginToken.isEmpty()) {
            std::cout << "令牌: " << autoLoginToken.left(16).toStdString() << "..." << std::endl;
        }
        
        // 验证自动登录令牌
        QString validatedUser = sessionManager_->validateAutoLoginToken(autoLoginToken);
        std::cout << "令牌验证: " << (!validatedUser.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
        std::cout << "验证用户: " << validatedUser.toStdString() << std::endl;
        
        // 验证无效令牌
        QString invalidUser = sessionManager_->validateAutoLoginToken("invalid-token");
        std::cout << "无效令牌验证(应该失败): " << (invalidUser.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 清除自动登录令牌
        bool clearResult = sessionManager_->clearAutoLoginToken(currentUser);
        std::cout << "令牌清除: " << (clearResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 验证已清除的令牌
        QString clearedUser = sessionManager_->validateAutoLoginToken(autoLoginToken);
        std::cout << "已清除令牌验证(应该失败): " << (clearedUser.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
    }
    
    void testSessionLimits() {
        std::cout << "\n[测试6] 会话限制测试..." << std::endl;
        
        QString testUser = "limituser";
        
        // 创建多个会话测试并发限制
        std::cout << "创建多个会话测试..." << std::endl;
        
        sessionManager_->destroySession(); // 清除当前会话
        
        QString session1 = sessionManager_->createSession(testUser, false, "Client1");
        std::cout << "会话1创建: " << (!session1.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
        
        QString session2 = sessionManager_->createSession(testUser, false, "Client2");
        std::cout << "会话2创建: " << (!session2.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 检查用户活跃会话数
        QStringList userSessions = sessionManager_->getUserActiveSessions(testUser);
        std::cout << "用户活跃会话数: " << userSessions.size() << std::endl;
        
        // 销毁用户所有会话
        int destroyedCount = sessionManager_->destroyUserSessions(testUser);
        std::cout << "销毁用户所有会话: " << destroyedCount << " 个" << std::endl;
        
        // 验证销毁结果
        QStringList remainingSessions = sessionManager_->getUserActiveSessions(testUser);
        std::cout << "销毁后剩余会话: " << remainingSessions.size() << " 个" << std::endl;
        
        // 检查总活跃会话数
        int totalActiveSessions = sessionManager_->getActiveSessionCount();
        std::cout << "系统总活跃会话数: " << totalActiveSessions << std::endl;
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("UnravelChat");
    app.setOrganizationName("unravel");
    
    SessionTester tester;
    QTimer::singleShot(100, &tester, &SessionTester::runTest);
    
    return app.exec();
}

#include "test_sessionmanager.moc"

