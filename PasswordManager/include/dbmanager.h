#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QObject>
#include <QSqlDatabase>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

    bool createDatabase();
    // Core functions that do NOT need UI access
    //bool createNewDatabase(const QString& dbFilePath, const QString& connectionName);
    //bool createPasswordTable(const QString& connectionName);
    //void closeConnection(const QString& connectionName);

private:


};
#endif // DBMANAGER_H
