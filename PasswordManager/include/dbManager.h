#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    bool openDecrypted(const QByteArray& decryptedData);
    int createentry(QString& name, QString& tag, QString& url, QString& username, QString& password, QString& notes);
    bool loadDecryptedData(const QByteArray &decryptedData);
    bool createDatabaseStructure();
    bool initializeMetaData();
    void closeAndLock();

    QSqlDatabase db() const;

private:
    DatabaseManager() = default;
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
