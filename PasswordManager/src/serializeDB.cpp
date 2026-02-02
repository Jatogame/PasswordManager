#include <QtSql/QSqlDriver>
#include <qsqldatabase.h>
#include <sqlite3.h>
#include "dbHeader.h"

QByteArray serializeDatabase() {
    // 1. Get the source handle (your active :memory: DB)
    QSqlDatabase db = QSqlDatabase::database("internal_db");
    sqlite3 *pInMemory = *static_cast<sqlite3**>(db.driver()->handle().data());

    // 2. On Windows/Linux, we can use "VACUUM INTO" to a temp memory-mapped file
    // Or simpler: use a temporary local file that we wipe immediately.
    // For a Password Manager, the data is usually very small (KB),
    // so we can use a QTemporaryFile which works on both OSs.

    QTemporaryFile tempFile;
    if (tempFile.open()) {
        QString path = tempFile.fileName();
        tempFile.close(); // Close so SQLite can write to it

        QSqlQuery query(db);
        // This command exports the whole DB to a single file on disk
        if (query.exec(QString("VACUUM INTO '%1'").arg(path))) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray data = file.readAll();
                file.close();
                return data; // Successfully exported
            }
        }
    }

    return QByteArray();
}
