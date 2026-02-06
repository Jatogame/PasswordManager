#include "dbHeader.h"

void closeAndLock() {
    // 1. Close the SQL connection
    {
        QSqlDatabase db = QSqlDatabase::database("internal_db");
        db.close();
    }
    // 2. Remove the connection from Qt's manager
    QSqlDatabase::removeDatabase("internal_db");

    // 3. Wipe the derived key and decrypted data
    sodium_memzero(runTime.derPass.data(), runTime.derPass.size());
    sodium_memzero(runTime.decryptedSQL.data(), runTime.decryptedSQL.size());
    runTime.derPass.clear();
    runTime.decryptedSQL.clear();
}
