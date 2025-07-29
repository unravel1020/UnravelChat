/**
 * @file UserDatabase.h
 * @author unravel
 * @date 2025-07-28
 */

#ifndef USERDATABASE_H
#define USERDATABASE_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStringList>
#include <memory>

struct UserInfo {
    QString username;
    QString email;
    QString displayName;
    QDateTime registeredDate;
    bool isActive;
    
    UserInfo() : isActive(false) {}
    
    UserInfo(const QString& user, const QString& mail, const QString& display = "")
        : username(user), email(mail), displayName(display.isEmpty() ? user : display),
          registeredDate(QDateTime::currentDateTime()), isActive(true) {}
    
    bool isValid() const {
        return !username.isEmpty() && !email.isEmpty();
    }
    
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["username"] = username;
        obj["email"] = email;
        obj["displayName"] = displayName;
        obj["registeredDate"] = registeredDate.toString(Qt::ISODate);
        obj["isActive"] = isActive;
        return obj;
    }
    
    static UserInfo fromJson(const QJsonObject& obj) {
        UserInfo info;
        info.username = obj["username"].toString();
        info.email = obj["email"].toString();
        info.displayName = obj["displayName"].toString();
        info.registeredDate = QDateTime::fromString(obj["registeredDate"].toString(), Qt::ISODate);
        info.isActive = obj["isActive"].toBool();
        return info;
    }
};

class UserDatabase : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<QSettings> settings_;
    QString databasePath_;

public:
    explicit UserDatabase(QObject* parent = nullptr);
    ~UserDatabase();
    
    bool initialize();
    bool saveUser(const UserInfo& userInfo, const QString& hashedPassword, const QString& salt);
    UserInfo getUserInfo(const QString& username) const;
    bool getUserPassword(const QString& username, QString& hashedPassword, QString& salt) const;
    bool userExists(const QString& username) const;
    bool emailExists(const QString& email) const;
    bool updateLastLoginTime(const QString& username);
    QStringList getAllUsernames() const;
    int getUserCount() const;
    bool deleteUser(const QString& username);
    QString getUsernameByEmail(const QString& email) const;
    QString getDatabasePath() const { return databasePath_; }

signals:
    void userSaved(const QString& username);
    void databaseError(const QString& operation, const QString& error);
};

#endif // USERDATABASE_H

