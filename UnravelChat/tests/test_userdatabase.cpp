#include "../include/auth/UserDatabase.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <iostream>

class SimpleTester : public QObject {
    Q_OBJECT

private:
    UserDatabase* database_;

public:
    explicit SimpleTester(QObject* parent = nullptr) : QObject(parent) {
        database_ = new UserDatabase(this);
        
        // 连接信号
        connect(database_, &UserDatabase::userSaved, 
                this, [](const QString& username) {
                    std::cout << "✓ 用户保存信号收到: " << username.toStdString() << std::endl;
                });
                
        connect(database_, &UserDatabase::databaseError,
                this, [](const QString& operation, const QString& error) {
                    std::cout << "✗ 数据库错误 [" << operation.toStdString() 
                              << "]: " << error.toStdString() << std::endl;
                });
    }

    void runTest() {
        std::cout << "========================================" << std::endl;
        std::cout << "开始UserDatabase基础功能测试..." << std::endl;
        std::cout << "========================================" << std::endl;
        
        // 测试1: 初始化
        std::cout << "\n[测试1] 数据库初始化..." << std::endl;
        bool initResult = database_->initialize();
        std::cout << "初始化结果: " << (initResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        if (initResult) {
            std::cout << "数据库路径: " << database_->getDatabasePath().toStdString() << std::endl;
            
            // 测试2: 用户保存
            std::cout << "\n[测试2] 用户保存..." << std::endl;
            UserInfo testUser("testuser001", "test@example.com", "测试用户");
            bool saveResult = database_->saveUser(testUser, "hashedPassword123", "salt456");
            std::cout << "保存结果: " << (saveResult ? "✓ 成功" : "✗ 失败") << std::endl;
            
            // 测试3: 用户存在检查
            std::cout << "\n[测试3] 用户存在检查..." << std::endl;
            bool existsResult = database_->userExists("testuser001");
            std::cout << "存在检查: " << (existsResult ? "✓ 成功" : "✗ 失败") << std::endl;
            
            // 测试4: 用户信息读取
            std::cout << "\n[测试4] 用户信息读取..." << std::endl;
            UserInfo retrievedUser = database_->getUserInfo("testuser001");
            std::cout << "读取结果: " << (retrievedUser.isValid() ? "✓ 成功" : "✗ 失败") << std::endl;
            
            if (retrievedUser.isValid()) {
                std::cout << "  用户名: " << retrievedUser.username.toStdString() << std::endl;
                std::cout << "  邮箱: " << retrievedUser.email.toStdString() << std::endl;
                std::cout << "  显示名: " << retrievedUser.displayName.toStdString() << std::endl;
                std::cout << "  注册时间: " << retrievedUser.registeredDate.toString().toStdString() << std::endl;
            }
            
            // 测试5: 邮箱存在检查
            std::cout << "\n[测试5] 邮箱存在检查..." << std::endl;
            bool emailExistsResult = database_->emailExists("test@example.com");
            std::cout << "邮箱存在: " << (emailExistsResult ? "✓ 成功" : "✗ 失败") << std::endl;
            
            // 测试6: 重复用户保存
            std::cout << "\n[测试6] 重复用户保存测试..." << std::endl;
            UserInfo duplicateUser("testuser001", "duplicate@example.com", "重复用户");
            bool duplicateResult = database_->saveUser(duplicateUser, "hash", "salt");
            std::cout << "重复保存(应该失败): " << (!duplicateResult ? "✓ 正确拒绝" : "✗ 错误接受") << std::endl;
        }
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "UserDatabase基础功能测试完成！" << std::endl;
        std::cout << "========================================" << std::endl;
        
        QTimer::singleShot(2000, qApp, &QCoreApplication::quit);
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("UnravelChat");
    app.setOrganizationName("unravel");
    
    SimpleTester tester;
    QTimer::singleShot(100, &tester, &SimpleTester::runTest);
    
    return app.exec();
}

#include "test_userdatabase.moc"

