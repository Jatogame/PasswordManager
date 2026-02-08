#include "dbManager.h"
#include "dbHeader.h"

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

QSqlDatabase DatabaseManager::db() const
{
    return m_db;
}

int DatabaseManager::createentry(QString& name, QString& tag, QString& url, QString& username, QString& password, QString& notes){
    //create new db entry and save the file

    if (!m_db.isOpen()) return -1;

    QSqlQuery query(m_db);

    query.prepare(
        "INSERT INTO passwords "
        "(name, tag, url, username, password, notes) "
        "VALUES (?, ?, ?, ?, ?, ?)"
        );
    query.addBindValue(name);
    query.addBindValue(tag);
    query.addBindValue(url);
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(notes);

    if (!query.exec()) {
        return -1;
    }
    return query.lastInsertId().toInt();
}

bool DatabaseManager::loadDecryptedData(const QByteArray &decryptedData){

    if (m_db.isValid()) {
        m_db.close();
        m_db = QSqlDatabase();            // drop handle
        QSqlDatabase::removeDatabase("internal_db");
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", "internal_db");
    m_db.setDatabaseName(":memory:");
    if (!m_db.open()) return false;

    // 2. Write decrypted DB to temp file
    QTemporaryFile tempFile;
    if (!tempFile.open())
        return false;

    tempFile.write(decryptedData);
    tempFile.flush();   // important
    tempFile.close();

    QSqlQuery query(m_db);

    // Create the tables exactly as you defined them
    if (!query.exec(
            "CREATE TABLE passwords ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT NOT NULL UNIQUE, "
            "tag TEXT, url TEXT, username TEXT, password TEXT, notes TEXT)"
            )) return false;

    if (!query.exec(
            "CREATE TABLE meta ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT, data TEXT)"
            )) return false;

    if (!query.exec(QString("ATTACH DATABASE '%1' AS temp_disk").arg(tempFile.fileName())))
        return false;

    if (!query.exec(
            "INSERT INTO main.passwords (id, name, tag, url, username, password, notes) "
            "SELECT id, name, tag, url, username, password, notes FROM temp_disk.passwords"
            )) return false;

    if (!query.exec(
            "INSERT INTO main.meta (id, name, data) "
            "SELECT id, name, data FROM temp_disk.meta"
            )) return false;

    // 5. Clean up
    query.exec("DETACH DATABASE temp_disk");
    tempFile.remove();

    return true;
}


//create the SQL structure
bool DatabaseManager::createDatabaseStructure() {

    if (!m_db.isOpen() && !m_db.open())
        return false;

    QSqlQuery query(m_db);

    // 2. Create 'passwords' table
    // Use AUTOINCREMENT for the index so you don't have to manage IDs manually
    bool success = query.exec(
        "CREATE TABLE passwords ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE, "
        "tag TEXT, "
        "url TEXT, "
        "username TEXT, "
        "password TEXT, "
        "notes TEXT"
        ")"
        );

    if (!success) return false;

    // 3. Create 'meta' table
    success = query.exec(
        "CREATE TABLE meta ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT, "
        "data TEXT"
        ")"
        );

    if (!success) return false;

    return true;
}

//add entry to check if database was decrypted correctly
bool DatabaseManager::initializeMetaData() {
    if (!m_db.isOpen())
        return false;

    QSqlQuery query(m_db);

    query.prepare("INSERT INTO meta (name, data) VALUES (?, ?)");
    query.addBindValue("check");
    query.addBindValue("JAPASS1");

    if (!query.exec()) {
        return false;
    }

    return true;
}

void DatabaseManager::closeAndLock() {
    // 1) Close the owned connection and drop the handle
    if (m_db.isValid()) {
        m_db.close();
        m_db = QSqlDatabase();   // IMPORTANT: release Qt's handle
    }

    // 2) Now it is safe to remove the connection by name
    if (QSqlDatabase::contains("internal_db")) {
        QSqlDatabase::removeDatabase("internal_db");
    }

    // 3) Wipe secrets (do this while buffers still exist)
    if (!runTime.derPass.isEmpty())
        sodium_memzero(runTime.derPass.data(), runTime.derPass.size());
    if (!runTime.decryptedSQL.isEmpty())
        sodium_memzero(runTime.decryptedSQL.data(), runTime.decryptedSQL.size());

    runTime.derPass.clear();
    runTime.decryptedSQL.clear();

    // Optional: reduce capacity (not guaranteed to wipe freed memory, but reduces exposure)
    runTime.derPass.squeeze();
    runTime.decryptedSQL.squeeze();
}
