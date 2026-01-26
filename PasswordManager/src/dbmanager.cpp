#include "dbmanager.h"
#include <QString>
#include <QFileDialog>
#include <QSqlDatabase>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent) // Initialize the QObject parent
{}

/*
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "newDatabaseConnection");
    db.setDatabaseName(path);

    if (!db.open()) {
        return;
    }
*/


bool DatabaseManager::createDatabase(const QString &filePath, const QString &masterPassword)
{
    //1. set definitions
    const QByteArray magic = "MYSAFE01";
    const int keySize = 32;
    const int iterations = 100000;

    const int saltSize = 16;
    const int nonceSize = 12;

    //2. generate Salt, Nonce
    QByteArray salt = generateRandom(saltSize);
    QByteArray nonce = generateRandom(nonceSize);

    //3. generate key from masterpassword (PBKDF2)
    QByteArray derivedKey(keySize, 0);
    if (!PKCS5_PBKDF2_HMAC(masterPassword.toUtf8().constData(), masterPassword.length(),
                           reinterpret_cast<const unsigned char*>(salt.constData()), salt.size(),
                           iterations, EVP_sha256(), keySize,
                           reinterpret_cast<unsigned char*>(derivedKey.data()))) {
        return false;
    }

    // 4. Verschlüsselung vorbereiten (ChaCha20)
    QByteArray encryptedData = encryptChaCha20(plainData, derivedKey, nonce);
    /*
    QByteArray encryptedData(plainData.size(), 0);
    int outLen = 0;
    int finalLen = 0;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_chacha20(), nullptr,
                       reinterpret_cast<unsigned char*>(derivedKey.data()),
                       reinterpret_cast<unsigned char*>(nonce.data()));

    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(encryptedData.data()), &outLen,
                      reinterpret_cast<const unsigned char*>(plainData.constData()), plainData.size());

    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encryptedData.data()) + outLen, &finalLen);
    EVP_CIPHER_CTX_free(ctx);
    */

    // 5. Datei schreiben
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    file.write(magic);      // Unverschlüsselter Header: Magic Bytes
    file.write(salt);       // Unverschlüsselter Header: Salt
    file.write(nonce);      // Unverschlüsselter Header: Nonce
    file.write(encryptedData); // Der verschlüsselte Rest (SQLite Payload)

    file.close();
    return true;
}

