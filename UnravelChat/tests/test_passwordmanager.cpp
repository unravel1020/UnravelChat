#include "../include/auth/PasswordManager.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <iostream>

class PasswordTester : public QObject {
    Q_OBJECT

private:
    PasswordManager* passwordManager_;

public:
    explicit PasswordTester(QObject* parent = nullptr) : QObject(parent) {
        passwordManager_ = new PasswordManager(this);
        
        // 连接信号
        connect(passwordManager_, &PasswordManager::passwordAnalyzed,
                this, [](const QString& masked, const PasswordAnalysis& analysis) {
                    std::cout << "密码分析完成: " << masked.toStdString() 
                              << " 强度: " << PasswordManager::getStrengthDescription(analysis.strength).toStdString()
                              << " 分数: " << analysis.score << std::endl;
                });
    }

    void runTest() {
        std::cout << "========================================" << std::endl;
        std::cout << "开始PasswordManager功能测试..." << std::endl;
        std::cout << "========================================" << std::endl;
        
        testSaltGeneration();
        testPasswordHashing();
        testPasswordStrengthAnalysis();
        testPasswordGeneration();
        testPasswordPolicy();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "PasswordManager功能测试完成！" << std::endl;
        std::cout << "========================================" << std::endl;
        
        QTimer::singleShot(2000, qApp, &QCoreApplication::quit);
    }

private:
    void testSaltGeneration() {
        std::cout << "\n[测试1] 盐值生成测试..." << std::endl;
        
        QString salt1 = PasswordManager::generateSalt();
        QString salt2 = PasswordManager::generateSalt();
        
        std::cout << "盐值1长度: " << salt1.length() << std::endl;
        std::cout << "盐值2长度: " << salt2.length() << std::endl;
        std::cout << "两个盐值不同: " << (salt1 != salt2 ? "✓ 成功" : "✗ 失败") << std::endl;
    }
    
    void testPasswordHashing() {
        std::cout << "\n[测试2] 密码哈希和验证测试..." << std::endl;
        
        QString password = "TestPassword123!";
        QString salt = PasswordManager::generateSalt();
        
        QString hash1 = PasswordManager::hashPassword(password, salt);
        QString hash2 = PasswordManager::hashPassword(password, salt);
        
        std::cout << "相同输入产生相同哈希: " << (hash1 == hash2 ? "✓ 成功" : "✗ 失败") << std::endl;
        
        bool verifyResult = PasswordManager::verifyPassword(password, hash1, salt);
        std::cout << "密码验证正确: " << (verifyResult ? "✓ 成功" : "✗ 失败") << std::endl;
        
        bool wrongVerify = PasswordManager::verifyPassword("WrongPassword", hash1, salt);
        std::cout << "错误密码验证失败: " << (!wrongVerify ? "✓ 成功" : "✗ 失败") << std::endl;
    }
    
    void testPasswordStrengthAnalysis() {
        std::cout << "\n[测试3] 密码强度分析测试..." << std::endl;
        
        QStringList testPasswords = {
            "123",                    // 非常弱
            "password",               // 弱  
            "Password123",            // 中等
            "StrongPass123!",         // 强
            "VeryStr0ng!P@ssw0rd2024" // 非常强
        };
        
        for (const QString& pwd : testPasswords) {
            PasswordAnalysis analysis = passwordManager_->analyzePassword(pwd);
            std::cout << "密码: " << QString("*").repeated(pwd.length()).toStdString()
                      << " 强度: " << PasswordManager::getStrengthDescription(analysis.strength).toStdString()
                      << " 分数: " << analysis.score
                      << " 符合策略: " << (analysis.meetsPolicy ? "是" : "否") << std::endl;
            
            if (!analysis.issues.isEmpty()) {
                std::cout << "  问题: ";
                for (const QString& issue : analysis.issues) {
                    std::cout << issue.toStdString() << "; ";
                }
                std::cout << std::endl;
            }
        }
    }
    
    void testPasswordGeneration() {
        std::cout << "\n[测试4] 密码生成测试..." << std::endl;
        
        // 测试基本密码生成
        QString generated1 = PasswordManager::generateSecurePassword(12);
        QString generated2 = PasswordManager::generateSecurePassword(12);
        
        std::cout << "生成密码1: " << generated1.toStdString() << std::endl;
        std::cout << "生成密码2: " << generated2.toStdString() << std::endl;
        std::cout << "两个密码不同: " << (generated1 != generated2 ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 测试不同字符类型的密码生成
        QString onlyLower = PasswordManager::generateSecurePassword(8, false, true, false, false);
        QString onlyDigits = PasswordManager::generateSecurePassword(8, false, false, true, false);
        
        std::cout << "仅小写字母: " << onlyLower.toStdString() << std::endl;
        std::cout << "仅数字: " << onlyDigits.toStdString() << std::endl;
        
        // 验证生成的密码强度
        PasswordAnalysis analysis = passwordManager_->analyzePassword(generated1);
        std::cout << "生成密码强度: " << PasswordManager::getStrengthDescription(analysis.strength).toStdString() << std::endl;
    }
    
    void testPasswordPolicy() {
        std::cout << "\n[测试5] 密码策略测试..." << std::endl;
        
        // 测试严格策略
        PasswordPolicy strictPolicy = PasswordPolicy::securePolicy();
        passwordManager_->setPasswordPolicy(strictPolicy);
        
        QString weakPassword = "123456";
        QString strongPassword = "MySecure123!Pass";
        
        bool weakValid = passwordManager_->validatePassword(weakPassword);
        bool strongValid = passwordManager_->validatePassword(strongPassword);
        
        std::cout << "弱密码验证(应该失败): " << (!weakValid ? "✓ 成功" : "✗ 失败") << std::endl;
        std::cout << "强密码验证(应该成功): " << (strongValid ? "✓ 成功" : "✗ 失败") << std::endl;
        
        // 测试密码创建辅助函数
        auto hashPair = PasswordManager::createPasswordHash(strongPassword);
        std::cout << "密码哈希创建: " << (!hashPair.first.isEmpty() && !hashPair.second.isEmpty() ? "✓ 成功" : "✗ 失败") << std::endl;
        
        bool hashVerify = PasswordManager::verifyPassword(strongPassword, hashPair.second, hashPair.first);
        std::cout << "哈希验证: " << (hashVerify ? "✓ 成功" : "✗ 失败") << std::endl;
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("UnravelChat");
    app.setOrganizationName("unravel");
    
    PasswordTester tester;
    QTimer::singleShot(100, &tester, &PasswordTester::runTest);
    
    return app.exec();
}

#include "test_passwordmanager.moc"
