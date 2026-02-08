#ifndef DBHEADER_H
#define DBHEADER_H
#include <QObject>
#include <QString>
#include <QFileDialog>
#include <QSqlDatabase>
#include <sodium.h>
#include <QSqlQuery>
#include <QSqlError>
#include <QTemporaryFile>

struct DbHeader {
    uint32_t version = 1;
    QByteArray salt;      // 16 Bytes
    uint32_t iterations = 10;
    uint32_t memoryCost = 2147483648; // 2GB, but carefull with overflow
    uint32_t parallelism = 1;
    QByteArray nonce;     // 12 Bytes
    QByteArray authTag;   // 16 Bytes
};

struct RunTimeData {
    QByteArray decryptedSQL;
    QByteArray encryptedSQL;
    QString filePath;
    //derived Password
    QByteArray derPass;
};

struct createDbHeader {
    const uint32_t version = 1;
    const uint32_t iterations = 10;
    const uint32_t memoryCost = 2147483648; // 2GB, but carefull with overflow
    const uint32_t parallelism = 1;
};

//make access to structs global
extern createDbHeader createMetaData;
extern RunTimeData runTime;
extern DbHeader metaData;

bool createFile();
bool loadEncryptedDb(const QString &path, const QByteArray &key);
void saveDatabase();
void encryptDB();
bool createDerPassword(QByteArray masterPassword);
bool derivePassword(QByteArray masterPassword);
QByteArray serializeDatabase();
bool createDatabaseStructure();
bool initializeMetaData();
void closeAndLock();
bool loadDatabase();
bool decryptDB();
bool loadDecryptedData(const QByteArray &decryptedData);
bool isDatabaseAuthentic();
QString generatePassword(int length, bool useLower, bool useUpper, bool useNumbers, bool useSpecial);
int createentry(QString name, QString tag, QString url, QString username, QString password, QString notes);

#endif // DBHEADER_H
