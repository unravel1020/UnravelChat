/**
 * @file PasswordManager.cpp
 * @author unravel
 * @date 2025-07-28
 * @framework Qt 6.x, C++17 Standard
 * @technology PBKDF2哈希算法, 安全随机数生成, 密码强度分析
 * @task UnravelChat第二阶段 - 密码安全管理实现
 */

#include "../../include/auth/PasswordManager.h"
#include <QtCore/QByteArray>
#include <QtCore/QStringList>
#include <cmath>

PasswordManager::PasswordManager(QObject* parent, const PasswordPolicy& policy)
    : QObject(parent), currentPolicy_(policy) {
}

// ==================== 密码哈希和验证 ====================

QString PasswordManager::generateSalt(int length) {
    QByteArray saltBytes;
    saltBytes.reserve(length);
    
    QRandomGenerator* rng = QRandomGenerator::system();
    
    for (int i = 0; i < length; ++i) {
        saltBytes.append(static_cast<char>(rng->bounded(0, 256)));
    }
    
    return saltBytes.toHex();
}

QString PasswordManager::hashPassword(const QString& password, const QString& salt, int iterations) {
    if (password.isEmpty() || salt.isEmpty()) {
        qWarning() << "PasswordManager::hashPassword: Empty password or salt";
        return QString();
    }
    
    iterations = qBound(1000, iterations, 100000);
    
    QByteArray passwordBytes = password.toUtf8();
    QByteArray saltBytes = QByteArray::fromHex(salt.toUtf8());
    
    QByteArray result = passwordBytes + saltBytes;
    
    for (int i = 0; i < iterations; ++i) {
        result = QCryptographicHash::hash(result, QCryptographicHash::Sha256);
    }
    
    return result.toHex();
}

bool PasswordManager::verifyPassword(const QString& password, 
                                   const QString& hashedPassword, 
                                   const QString& salt, 
                                   int iterations) {
    if (password.isEmpty() || hashedPassword.isEmpty() || salt.isEmpty()) {
        return false;
    }
    
    QString computedHash = hashPassword(password, salt, iterations);
    
    if (computedHash.length() != hashedPassword.length()) {
        return false;
    }
    
    bool result = true;
    for (int i = 0; i < computedHash.length(); ++i) {
        if (computedHash.at(i) != hashedPassword.at(i)) {
            result = false;
        }
    }
    
    return result;
}

QPair<QString, QString> PasswordManager::createPasswordHash(const QString& password, int iterations) {
    QString salt = generateSalt();
    QString hash = hashPassword(password, salt, iterations);
    return qMakePair(salt, hash);
}

// ==================== 密码强度分析 ====================

PasswordAnalysis PasswordManager::analyzePassword(const QString& password) {
    PasswordAnalysis analysis;
    
    if (password.isEmpty()) {
        analysis.issues.append("密码不能为空");
        return analysis;
    }
    
    analysis.score = calculatePasswordScore(password);
    analysis.strength = getPasswordStrength(analysis.score);
    analysis.issues = getPolicyViolations(password);
    analysis.meetsPolicy = analysis.issues.isEmpty();
    
    QString maskedPassword = QString("*").repeated(password.length());
    
    return analysis;
}

bool PasswordManager::validatePassword(const QString& password) const {
    return getPolicyViolations(password).isEmpty();
}

int PasswordManager::calculatePasswordScore(const QString& password) {
    if (password.isEmpty()) {
        return 0;
    }
    
    int score = 0;
    
    // 基础分数：长度
    int length = password.length();
    if (length >= 8) score += 10;
    if (length >= 12) score += 10;
    if (length >= 16) score += 10;
    if (length >= 20) score += 10;
    
    // 字符类型分析
    auto [hasUpper, hasLower, hasDigit, hasSpecial] = analyzeCharacterTypes(password);
    
    if (hasUpper) score += 10;
    if (hasLower) score += 10;
    if (hasDigit) score += 10;
    if (hasSpecial) score += 15;
    
    // 字符种类多样性奖励
    int typeCount = (hasUpper ? 1 : 0) + (hasLower ? 1 : 0) + 
                   (hasDigit ? 1 : 0) + (hasSpecial ? 1 : 0);
    score += typeCount * 5;
    
    // 检查重复字符（降分）
    QMap<QChar, int> charCount;
    for (const QChar& ch : password) {
        charCount[ch]++;
    }
    
    int maxRepeat = 0;
    for (auto it = charCount.begin(); it != charCount.end(); ++it) {
        maxRepeat = qMax(maxRepeat, it.value());
    }
    
    if (maxRepeat > 2) {
        score -= (maxRepeat - 2) * 5;
    }
    
    // 检查常见弱密码
    if (isCommonWeakPassword(password)) {
        score -= 30;
    }
    
    return qBound(0, score, 100);
}

PasswordStrength PasswordManager::getPasswordStrength(int score) {
    if (score < 20) return PasswordStrength::VeryWeak;
    if (score < 40) return PasswordStrength::Weak;
    if (score < 60) return PasswordStrength::Medium;
    if (score < 80) return PasswordStrength::Strong;
    return PasswordStrength::VeryStrong;
}

QString PasswordManager::getStrengthDescription(PasswordStrength strength) {
    switch (strength) {
        case PasswordStrength::VeryWeak: return "非常弱";
        case PasswordStrength::Weak: return "弱";
        case PasswordStrength::Medium: return "中等";
        case PasswordStrength::Strong: return "强";
        case PasswordStrength::VeryStrong: return "非常强";
    }
    return "未知";
}

// ==================== 密码生成 ====================

QString PasswordManager::generateSecurePassword(int length, 
                                               bool includeUppercase, 
                                               bool includeLowercase, 
                                               bool includeDigits, 
                                               bool includeSpecialChars) {
    if (length <= 0) {
        return QString();
    }
    
    QString charset;
    
    if (includeLowercase) {
        charset += "abcdefghijklmnopqrstuvwxyz";
    }
    
    if (includeUppercase) {
        charset += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    }
    
    if (includeDigits) {
        charset += "0123456789";
    }
    
    if (includeSpecialChars) {
        charset += "!@#$%^&*()_+-=[]{}|;:,.<>?";
    }
    
    if (charset.isEmpty()) {
        return QString();
    }
    
    QString password;
    password.reserve(length);
    
    QRandomGenerator* rng = QRandomGenerator::system();
    
    // 确保至少包含每种要求的字符类型
    if (includeUppercase && length > 0) {
        password += "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rng->bounded(26)];
    }
    if (includeLowercase && length > 1) {
        password += "abcdefghijklmnopqrstuvwxyz"[rng->bounded(26)];
    }
    if (includeDigits && length > 2) {
        password += "0123456789"[rng->bounded(10)];
    }
    if (includeSpecialChars && length > 3) {
        password += "!@#$%^&*"[rng->bounded(8)];
    }
    
    // 填充剩余长度
    while (password.length() < length) {
        password += charset[rng->bounded(charset.length())];
    }
    
    // 随机打乱字符顺序
    for (int i = password.length() - 1; i > 0; --i) {
        int j = rng->bounded(i + 1);
        QChar temp = password[i];
        password[i] = password[j];
        password[j] = temp;
    }
    
    return password;
}

// ==================== 策略管理 ====================

void PasswordManager::setPasswordPolicy(const PasswordPolicy& policy) {
    currentPolicy_ = policy;
    emit policyChanged(policy);
}

// ==================== 私有辅助方法 ====================

std::tuple<bool, bool, bool, bool> PasswordManager::analyzeCharacterTypes(const QString& password) {
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    for (const QChar& ch : password) {
        if (ch.isUpper()) {
            hasUpper = true;
        } else if (ch.isLower()) {
            hasLower = true;
        } else if (ch.isDigit()) {
            hasDigit = true;
        } else {
            hasSpecial = true;
        }
    }
    
    return std::make_tuple(hasUpper, hasLower, hasDigit, hasSpecial);
}

QStringList PasswordManager::getPolicyViolations(const QString& password) const {
    QStringList violations;
    
    if (password.length() < currentPolicy_.minLength) {
        violations.append(QString("密码长度至少需要%1个字符").arg(currentPolicy_.minLength));
    }
    
    auto [hasUpper, hasLower, hasDigit, hasSpecial] = analyzeCharacterTypes(password);
    
    if (currentPolicy_.requireUppercase && !hasUpper) {
        violations.append("密码必须包含大写字母");
    }
    
    if (currentPolicy_.requireLowercase && !hasLower) {
        violations.append("密码必须包含小写字母");
    }
    
    if (currentPolicy_.requireDigits && !hasDigit) {
        violations.append("密码必须包含数字");
    }
    
    if (currentPolicy_.requireSpecialChars && !hasSpecial) {
        violations.append("密码必须包含特殊字符");
    }
    
    return violations;
}

// ==================== 实用工具方法 ====================

bool PasswordManager::isCommonWeakPassword(const QString& password) {
    static const QStringList commonPasswords = {
        "123456", "password", "123456789", "12345678", "12345",
        "1234567", "1234567890", "qwerty", "abc123", "Password",
        "password123", "admin", "letmein", "welcome", "monkey"
    };
    
    QString lowerPassword = password.toLower();
    
    for (const QString& weak : commonPasswords) {
        if (lowerPassword == weak.toLower()) {
            return true;
        }
    }
    
    return false;
}

