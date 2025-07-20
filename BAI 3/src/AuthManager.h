#pragma once
#include <QString>
#include <QCryptographicHash>
class AuthManager {
public:
    static AuthManager& instance();

    bool registerUser(const QString& username, const QString& password);
    bool validate(const QString& username, const QString& password);
    bool isUnique(const QString& username);
    QString encryptMessage(const QString& chatKey, const QString& plainText);
    QString decryptMessage(const QString& chatKey, const QString& cipherText);
private:
    AuthManager() {}
    QString generateSalt(int length = 16);
    QByteArray hashPassword(const QString& password, const QString& salt, int iterations = 100000);

    const QString FILE_NAME = "Username.txt";
};


