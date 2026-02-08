#include "dbHeader.h"

bool createFile(){
    if (createDatabaseStructure() != true) return false;
    if (initializeMetaData() != true) return false;
    saveDatabase();
    closeAndLock();
    return true;
}

//create the SQL structure
bool createDatabaseStructure() {
    // 1. Create a connection to an in-memory database
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "internal_db");
    db.setDatabaseName(":memory:");

    if (!db.open()) {
        //qDebug() << "Error opening memory database:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

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
bool initializeMetaData() {
    QSqlDatabase db = QSqlDatabase::database("internal_db");
    QSqlQuery query(db);

    query.prepare("INSERT INTO meta (name, data) VALUES (?, ?)");
    query.addBindValue("check");
    query.addBindValue("JAPASS1");

    if (!query.exec()) {
        return false;
    }

    return true;
}
