#include "dbHeader.h"

bool decryptDB(){
    // 1. Safety Check
    if (runTime.encryptedSQL.size() < 16) return false;

    // 2. Prepare the output buffer
    // The plaintext will be exactly 16 bytes smaller than the encrypted blob
    QByteArray decryptedData;
    decryptedData.resize(runTime.encryptedSQL.size() - 16);
    unsigned long long outLen;

    // 3. Decrypt
    // Libsodium will automatically pull the 16-byte tag from the end of encryptedSQL
    int result = crypto_aead_chacha20poly1305_ietf_decrypt(
        reinterpret_cast<unsigned char*>(decryptedData.data()),
        &outLen,
        nullptr, // No secret nonce
        reinterpret_cast<const unsigned char*>(runTime.encryptedSQL.constData()),
        runTime.encryptedSQL.size(),
        nullptr, 0, // No additional authenticated data
        reinterpret_cast<const unsigned char*>(metaData.nonce.constData()),
        reinterpret_cast<const unsigned char*>(runTime.derPass.constData())
        );

    if (result != 0) {
        qDebug() << "Decryption failed! The file was tampered with or the password is wrong.";
        return false;
    }

    // 4. Load the raw bytes into your in-memory SQLite
    // Using QTemporaryFile
    return restoreFromBytes(decryptedData);
}

bool isDatabaseAuthentic() {
    QSqlDatabase db = QSqlDatabase::database("internal_db");
    QSqlQuery query("SELECT data FROM meta WHERE name = 'check'", db);

    if (query.next()) {
        QString checkValue = query.value(0).toString();
        if (checkValue == "JAPASS1") {
            qDebug() << "Database verified! Welcome back.";
            return true;
        }
    }

    qDebug() << "Database structure is okay, but the 'check' record is missing or wrong.";
    return false;
}
