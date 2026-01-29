#ifndef DBHEADER_H
#define DBHEADER_H
#include <QObject>
#include <QString>
#include <QFileDialog>
#include <QSqlDatabase>

struct DbHeader {
    const uint16_t version = 1;
    QByteArray salt;      // 16 Bytes
    const uint32_t iterations = 20;
    const uint32_t memoryCost = 2147483648; // 2GB, but carefull with overflow
    const uint32_t parallelism = 1;
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
//make access to structs global
extern RunTimeData runTime;
extern DbHeader metaData;

bool createFile();
bool loadEncryptedDb(const QString &path, const QByteArray &key);
void saveDatabase();
void encryptDB();
void derivePassword(QByteArray masterPassword);

#endif // DBHEADER_H
