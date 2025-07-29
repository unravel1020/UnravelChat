/**
 * @file PasswordManager.h
 * @author unravel
 * @date 2025-07-28
 * @framework Qt 6.x, C++17 Standard
 * @technology QCryptographicHash, QRandomGenerator, 密码学安全
 * @task UnravelChat第二阶段 - 密码安全管理
 */

#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRandomGenerator>
#include <QtCore/QRegularExpression>
#include <QtCore/QDebug>

enum class PasswordStrength {
    VeryWeak = 0,
    Weak = 1,
    Medium = 2,
    Strong = 3,
    VeryStrong = 4
};

struct PasswordPolicy {
    int minLength = 8;
    bool requireUppercase = true;
    bool requireLowercase = true;
    bool requireDigits = true;
    bool requireSpecialChars = true;
    
    static PasswordPolicy securePolicy() {
        PasswordPolicy policy;
        policy.minLength = 12;
        return policy;
    }
};

struct PasswordAnalysis {
    PasswordStrength strength;
    int score;
    QStringList issues;
    bool meetsPolicy;
    
    PasswordAnalysis() : strength(PasswordStrength::VeryWeak), score(0), meetsPolicy(false) {}
};

class PasswordManager : public QObject {
    Q_OBJECT

private:
    PasswordPolicy currentPolicy_;
    static constexpr int DEFAULT_SALT_LENGTH = 32;
    static constexpr int DEFAULT_HASH_ITERATIONS = 10000;

public:
    explicit PasswordManager(QObject* parent = nullptr, 
                           const PasswordPolicy& policy = PasswordPolicy::securePolicy());
    
    // 密码哈希和验证
    static QString generateSalt(int length = DEFAULT_SALT_LENGTH);
    static QString hashPassword(const QString& password, const QString& salt, int iterations = DEFAULT_HASH_ITERATIONS);
    static bool verifyPassword(const QString& password, const QString& hashedPassword, const QString& salt, int iterations = DEFAULT_HASH_ITERATIONS);
    static QPair<QString, QString> createPasswordHash(const QString& password, int iterations = DEFAULT_HASH_ITERATIONS);
    
    // 密码强度分析 - 注意：这里去掉了const
    PasswordAnalysis analyzePassword(const QString& password);
    bool validatePassword(const QString& password) const;
    static int calculatePasswordScore(const QString& password);
    static PasswordStrength getPasswordStrength(int score);
    static QString getStrengthDescription(PasswordStrength strength);
    
    // 密码生成
    static QString generateSecurePassword(int length = 16, bool includeUppercase = true, bool includeLowercase = true, bool includeDigits = true, bool includeSpecialChars = true);
    
    // 策略管理
    void setPasswordPolicy(const PasswordPolicy& policy);
    const PasswordPolicy& getPasswordPolicy() const { return currentPolicy_; }
    
    // 添加缺失的静态函数声明
    static bool isCommonWeakPassword(const QString& password);

private:
    static std::tuple<bool, bool, bool, bool> analyzeCharacterTypes(const QString& password);
    QStringList getPolicyViolations(const QString& password) const;

signals:
    void policyChanged(const PasswordPolicy& newPolicy);
    void passwordAnalyzed(const QString& maskedPassword, const PasswordAnalysis& analysis);
};

#endif // PASSWORDMANAGER_H

