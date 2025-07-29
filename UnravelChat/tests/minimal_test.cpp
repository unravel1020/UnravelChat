#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QDebug>
#include <iostream>

// 最小化的PasswordManager类，不使用QObject
class MinimalPasswordManager {
public:
    static int calculatePasswordScore(const QString& password) {
        if (password.isEmpty()) return 0;
        
        int score = 0;
        
        // 基础长度分数
        if (password.length() >= 8) score += 20;
        if (password.length() >= 12) score += 20;
        
        // 字符类型检查
        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        
        for (const QChar& ch : password) {
            if (ch.isUpper()) hasUpper = true;
            else if (ch.isLower()) hasLower = true;
            else if (ch.isDigit()) hasDigit = true;
            else hasSpecial = true;
        }
        
        if (hasUpper) score += 15;
        if (hasLower) score += 15;
        if (hasDigit) score += 15;
        if (hasSpecial) score += 15;
        
        return score;
    }
    
    static bool isPasswordStrong(const QString& password) {
        return calculatePasswordScore(password) >= 60;
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "最小化测试开始..." << std::endl;
    
    // 测试密码分析
    QString testPassword = "SimplePass123!";
    std::cout << "测试密码: " << testPassword.toStdString() << std::endl;
    
    int score = MinimalPasswordManager::calculatePasswordScore(testPassword);
    bool isStrong = MinimalPasswordManager::isPasswordStrong(testPassword);
    
    std::cout << "密码分数: " << score << std::endl;
    std::cout << "密码强度: " << (isStrong ? "强" : "弱") << std::endl;
    std::cout << "测试完成，没有死锁!" << std::endl;
    
    return 0;
}

