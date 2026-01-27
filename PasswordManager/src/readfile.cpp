#include <QSqlDatabase>
#include <QFile>
#include <sqlite3.h>
#include <QtSql/QSqlDriver>

bool loadEncryptedDb(const QString &path, const QByteArray &key) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;

    // 1. skip header (z.B. 64 Bytes)
    file.seek(64);
    QByteArray encryptedData = file.readAll();

    // 2. decrypt (Pseudo-Code)
    QByteArray decryptedData = MyCrypto::decrypt(encryptedData, key);

    // 3. Qt SQLite connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) return false;

    // 4. get SQLite-Handle, to load into RAM
    QVariant v = db.driver()->handle();
    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*") == 0) {
        sqlite3 *handle = *static_cast<sqlite3**>(v.data());

        // Deserialisieren: Schiebt die Bytes direkt in die Memory-DB
        // SQLITE_DESERIALIZE_FREEONCLOSE sorgt dafür, dass SQLite das Array löscht
        unsigned char* sqliteBuffer = static_cast<unsigned char*>(sqlite3_malloc(decryptedData.size()));
        memcpy(sqliteBuffer, decryptedData.data(), decryptedData.size());

        sqlite3_deserialize(handle, "main", sqliteBuffer, decryptedData.size(),
                            decryptedData.size(), SQLITE_DESERIALIZE_FREEONCLOSE);
    }
    return true;
}
