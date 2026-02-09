#include <QtSql/QSqlDriver>
#include <qsqldatabase.h>
#include <sqlite3.h>
#include "dbHeader.h"
#include "dbManager.h"

QByteArray serializeDatabase() {
    //Get the source handle (the active :memory: DB)
    QSqlDatabase db = DatabaseManager::instance().db();
    //sqlite3 *pInMemory = *static_cast<sqlite3**>(db.driver()->handle().data());

    //temporary local file, wiped immediately

    QTemporaryFile tempFile;
    if (tempFile.open()) {
        QString path = tempFile.fileName();
        tempFile.close(); //close so SQLite can write to it

        QSqlQuery query(db);
        //exports the whole DB to a single file on disk
        if (query.exec(QString("VACUUM INTO '%1'").arg(path))) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray data = file.readAll();
                file.close();
                return data; //successfully exported
            }
        }
    }

    return QByteArray();
}
