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



