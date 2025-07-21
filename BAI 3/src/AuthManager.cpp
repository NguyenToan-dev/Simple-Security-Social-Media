#include "AuthManager.h"
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QByteArray>
#include <sodium.h>  // thư viện giúp mã hoá nội dung

// Hàm tạo khoá mã hoá từ chuỗi `chatKey` kết hợp với chuỗi bí mật để đảm bảo thống nhất
static QByteArray deriveChatKey(const QString& chatKey) {
    QByteArray raw = (chatKey + QString("SomeServerSecret")).toUtf8();
    return QCryptographicHash::hash(raw, QCryptographicHash::Sha256);
}

// Singleton: đảm bảo chỉ có một thể hiện duy nhất của AuthManager trong toàn chương trình
AuthManager& AuthManager::instance() {
    static AuthManager inst;
    return inst;
}

// Tạo chuỗi salt ngẫu nhiên gồm `length` ký tự thường từ a–z
QString AuthManager::generateSalt(int length) {
    QByteArray salt;
    for (int i = 0; i < length; ++i)
        salt.append(char('a' + QRandomGenerator::global()->bounded(26)));
    return QString::fromUtf8(salt);
}

// Băm mật khẩu kết hợp với salt, thực hiện lặp nhiều lần để tăng độ khó khi tấn công
QByteArray AuthManager::hashPassword(const QString& password, const QString& salt, int iterations) {
    QByteArray result = (password + salt).toUtf8();
    for (int i = 0; i < iterations; ++i)
        result = QCryptographicHash::hash(result, QCryptographicHash::Sha256);
    return result.toHex();// chuyển kết quả thành dạng hex để dễ lưu trữ
}
// Kiểm tra username có trùng không (có trong file hay chưa)
bool AuthManager::isUnique(const QString& username) {
    QFile file(FILE_NAME);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return true; // nếu không đọc được file thì xem như không trùng

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.split(':').value(0) == username)
            return false;
    }
    return true;
}

// Đăng ký người dùng mới với username và password
bool AuthManager::registerUser(const QString& username, const QString& password) {
    if (!isUnique(username))
        return false;// từ chối nếu tên đã tồn tại

    QString salt = generateSalt();// tạo salt mới
    QByteArray hashed = hashPassword(password, salt);// băm mật khẩu

    QFile file(FILE_NAME);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << username << ":" << salt << ":" << hashed << "\n";// lưu dòng gồm: tên:salt:hash
    return true;
}
// Xác thực thông tin đăng nhập bằng cách so sánh hash mật khẩu
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
            QByteArray hashedInput = hashPassword(password, parts[1]);// hash với salt
            return hashedInput == parts[2].toUtf8();// so sánh với hash lưu
        }
    }
    return false;// không tìm thấy user
}
// Mã hoá một tin nhắn văn bản với khoá chatKey, trả về dạng Base64
QString AuthManager::encryptMessage(const QString& chatKey, const QString& plainText) {
    // 1. Lấy key 32 byte từ deriveChatKey
    QByteArray shaKey = deriveChatKey(chatKey);
    // libsodium cần key đúng độ dài crypto_secretbox_KEYBYTES (32)
    unsigned char key[crypto_secretbox_KEYBYTES];
    memcpy(key, shaKey.constData(), crypto_secretbox_KEYBYTES);

    // 2. Tạo nonce ngẫu nhiên
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    // 3. Chuẩn bị buffer cho ciphertext (message + MAC)
    QByteArray plain = plainText.toUtf8();
    int clen = plain.size() + crypto_secretbox_MACBYTES;
    QByteArray cipher;
    cipher.resize(clen);

    // 4. Mã hóa
    crypto_secretbox_easy(
        reinterpret_cast<unsigned char*>(cipher.data()),
        reinterpret_cast<const unsigned char*>(plain.constData()),
        (unsigned long long)plain.size(),
        nonce,
        key
    );

    // 5. Kết hợp nonce + ciphertext rồi base64
    QByteArray out;
    out.append(reinterpret_cast<char*>(nonce), sizeof nonce);
    out.append(cipher);
    return out.toBase64();
}
// Giải mã tin nhắn đã mã hoá base64 với chatKey
QString AuthManager::decryptMessage(const QString& chatKey, const QString& cipherText) {
    // 1. Decode base64
    QByteArray in = QByteArray::fromBase64(cipherText.toUtf8());
    if (in.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES)
        return QString();

    // 2. Tách nonce và ciphertext
    const unsigned char* nonce = reinterpret_cast<const unsigned char*>(in.constData());
    const unsigned char* ctext = reinterpret_cast<const unsigned char*>(in.constData() + crypto_secretbox_NONCEBYTES);
    qsizetype ctext_len = in.size() - crypto_secretbox_NONCEBYTES;

    // 3. Lấy key 32 byte
    QByteArray shaKey = deriveChatKey(chatKey);
    unsigned char key[crypto_secretbox_KEYBYTES];
    memcpy(key, shaKey.constData(), crypto_secretbox_KEYBYTES);

    // 4. Buffer cho plain
    QByteArray plain;
    plain.resize(ctext_len - crypto_secretbox_MACBYTES);

    // 5. Giải mã, kiểm tra integrity
    if (crypto_secretbox_open_easy(
        reinterpret_cast<unsigned char*>(plain.data()),
        ctext,
        (unsigned long long)ctext_len,
        nonce,
        key) != 0) {
        // xác thực thất bại
        return QString();
    }

    return QString::fromUtf8(plain);
}
