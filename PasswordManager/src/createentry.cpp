#include "dbHeader.h"

int createentry(QString name, QString tag, QString url, QString username, QString password, QString notes){
    //create new db entry and save the file
    QSqlDatabase db = QSqlDatabase::database("internal_db");
    QSqlQuery query(db);

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
        //error
    }
    return 0;
}
