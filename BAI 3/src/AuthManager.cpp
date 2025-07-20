#include "AuthManager.h"
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QByteArray>

AuthManager& AuthManager::instance() {
    static AuthManager inst;
    return inst;
}

QString AuthManager::generateSalt(int length) {
    QByteArray salt;
    for (int i = 0; i < length; ++i)
        salt.append(char('a' + QRandomGenerator::global()->bounded(26)));
    return QString::fromUtf8(salt);
}

QByteArray AuthManager::hashPassword(const QString& password, const QString& salt, int iterations) {
    QByteArray result = (password + salt).toUtf8();
    for (int i = 0; i < iterations; ++i)
        result = QCryptographicHash::hash(result, QCryptographicHash::Sha256);
    return result.toHex();
}

bool AuthManager::isUnique(const QString& username) {
    QFile file(FILE_NAME);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return true;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.split(':').value(0) == username)
            return false;
    }
    return true;
}

bool AuthManager::registerUser(const QString& username, const QString& password) {
    if (!isUnique(username))
        return false;

    QString salt = generateSalt();
    QByteArray hashed = hashPassword(password, salt);

    QFile file(FILE_NAME);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << username << ":" << salt << ":" << hashed << "\n";
    return true;
}

bool AuthManager::validate(const QString& username, const QString& password) {
    QFile file(FILE_NAME);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(':');
        if (parts.size() != 3) continue;
        if (parts[0] == username) {
            QByteArray hashedInput = hashPassword(password, parts[1]);
            return hashedInput == parts[2].toUtf8();
        }
    }
    return false;
}

static QByteArray deriveChatKey(const QString& chatKey) {
    // bạn có thể thay deriveKey(username) bằng SHA256(chatKey + secret)
    QByteArray raw = (chatKey + QString("SomeServerSecret")).toUtf8();
    return QCryptographicHash::hash(raw, QCryptographicHash::Sha256);
}

QString AuthManager::encryptMessage(const QString& chatKey, const QString& plainText) {
    QByteArray key = deriveChatKey(chatKey);
    QByteArray ba = plainText.toUtf8();
    for (int i = 0; i < ba.size(); ++i)
        ba[i] = ba[i] ^ key[i % key.size()];
    return ba.toBase64();
}

QString AuthManager::decryptMessage(const QString& chatKey, const QString& cipherText) {
    QByteArray key = deriveChatKey(chatKey);
    QByteArray ba = QByteArray::fromBase64(cipherText.toUtf8());
    for (int i = 0; i < ba.size(); ++i)
        ba[i] = ba[i] ^ key[i % key.size()];
    return QString::fromUtf8(ba);
}
