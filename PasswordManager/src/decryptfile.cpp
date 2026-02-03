#include "dbHeader.h"
#include <QtSql/QSqlDriver>
#include <qsqldatabase.h>
#include <sqlite3.h>

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
    if (loadDecryptedData(decryptedData) != true){
        return false;
    }
    sodium_memzero(decryptedData.data(), decryptedData.size());

    return true;
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

bool loadDecryptedData(const QByteArray &decryptedData){
    // 1. Re-initialize the Qt SQL connection
    // This creates a brand new, empty database in RAM
    if (QSqlDatabase::contains("internal_db")) {
        QSqlDatabase::removeDatabase("internal_db");
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "internal_db");
    db.setDatabaseName(":memory:");

    if (!db.open()) {
        qDebug() << "Could not open memory database!";
        return false;
    }

    // 2. Create the temporary file to bridge the data
    QTemporaryFile tempFile;
    if (!tempFile.open()) return false;
    tempFile.write(decryptedData);
    tempFile.close();

    // 3. Prepare the RAM database with the correct rules (Schema)
    QSqlQuery query(db);

    // Create the tables exactly as you defined them
    query.exec(
        "CREATE TABLE passwords ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "tag TEXT, "
        "url TEXT, "
        "username TEXT, "
        "password TEXT, "
        "notes TEXT"
        ")"
        );

    query.exec(
        "CREATE TABLE meta ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "data TEXT"
        ")"
        );

    // Now attach the decrypted file
    if (!query.exec(QString("ATTACH DATABASE '%1' AS temp_disk").arg(tempFile.fileName()))) {
        return false;
    }

    // 4. Import the data into the already-created tables
    // We specify the target columns to be safe
    query.exec("INSERT INTO main.passwords SELECT * FROM temp_disk.passwords");
    query.exec("INSERT INTO main.meta SELECT * FROM temp_disk.meta");

    // 5. Clean up
    query.exec("DETACH DATABASE temp_disk");

    return true;
}
