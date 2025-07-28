#include "../../include/auth/UserDatabase.h"
#include <QtCore/QDir>
#include <QtCore/QDebug>

UserDatabase::UserDatabase(QObject* parent)
    : QObject(parent), settings_(nullptr) {
}

UserDatabase::~UserDatabase() {
    if (settings_) {
        settings_->sync();
    }
}

bool UserDatabase::initialize() {
    try {
        // 创建应用数据目录
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir;
        if (!dir.exists(appDataPath)) {
            if (!dir.mkpath(appDataPath)) {
                emit databaseError("Initialize", "Failed to create data directory");
                return false;
            }
        }
        
        // 设置数据库路径
        databasePath_ = appDataPath + "/userdata.ini";
        
        // 创建QSettings对象
        settings_ = std::make_unique<QSettings>(databasePath_, QSettings::IniFormat);
        
        
        if (!settings_->isWritable()) {
            emit databaseError("Initialize", "Database file is not writable");
            return false;
        }
        
        qDebug() << "UserDatabase initialized at:" << databasePath_;
        return true;
        
    } catch (const std::exception& e) {
        emit databaseError("Initialize", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

bool UserDatabase::saveUser(const UserInfo& userInfo, const QString& hashedPassword, const QString& salt) {
    if (!settings_) {
        emit databaseError("SaveUser", "Database not initialized");
        return false;
    }
    
    if (!userInfo.isValid() || hashedPassword.isEmpty() || salt.isEmpty()) {
        emit databaseError("SaveUser", "Invalid user information");
        return false;
    }
    
    if (userExists(userInfo.username)) {
        emit databaseError("SaveUser", "User already exists");
        return false;
    }
    
    try {
        QString userGroup = QString("Users/%1").arg(userInfo.username);
        
        settings_->beginGroup(userGroup);
        
        // 保存用户信息
        QJsonDocument doc(userInfo.toJson());
        settings_->setValue("UserInfo", QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
        settings_->setValue("PasswordHash", hashedPassword);
        settings_->setValue("PasswordSalt", salt);
        
        settings_->endGroup();
        settings_->sync();
        
        emit userSaved(userInfo.username);
        qDebug() << "User saved:" << userInfo.username;
        return true;
        
    } catch (const std::exception& e) {
        emit databaseError("SaveUser", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

UserInfo UserDatabase::getUserInfo(const QString& username) const {
    if (!settings_ || username.isEmpty()) {
        return UserInfo();
    }
    
    QString userGroup = QString("Users/%1").arg(username);
    
    settings_->beginGroup(userGroup);
    QString userInfoJson = settings_->value("UserInfo").toString();
    settings_->endGroup();
    
    if (userInfoJson.isEmpty()) {
        return UserInfo();
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(userInfoJson.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return UserInfo();
    }
    
    return UserInfo::fromJson(doc.object());
}

bool UserDatabase::userExists(const QString& username) const {
    if (!settings_ || username.isEmpty()) {
        return false;
    }
    
    return settings_->contains(QString("Users/%1/UserInfo").arg(username));
}

bool UserDatabase::emailExists(const QString& email) const {
    if (!settings_ || email.isEmpty()) {
        return false;
    }
    
    // 简化实现：遍历所有用户检查邮箱
    settings_->beginGroup("Users");
    QStringList users = settings_->childGroups();
    settings_->endGroup();
    
    for (const QString& user : users) {
        UserInfo userInfo = getUserInfo(user);
        if (userInfo.email == email) {
            return true;
        }
    }
    
    return false;
}

