#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include "dbHeader.h"

void saveDatabase() {

    //take data from struct
    DbHeader metaData;
    RunTimeData runTime;

    //encrypt data
    QByteArray encryptedSQLite;

    encryptDB();

    QFile file(runTime.filePath);
    if (!file.open(QIODevice::WriteOnly)) return;

    QDataStream out(&file);
    out.setByteOrder(QDataStream::BigEndian);

    // 1. Magic ID (Tag 0x01)
    out << (quint8)0x01 << (quint16)8;
    file.write("JAPASSDB", 8);

    // 2. Version (Tag 0x02)
    out << (quint8)0x02 << (quint16)2 << metaData.version;

    // 3. Argon2 Salt (Tag 0x03)
    out << (quint8)0x03 << (quint16)metaData.salt.size();
    file.write(metaData.salt);

    // 4. Argon2 Params (Tag 0x04) - 12 Bytes
    out << (quint8)0x04 << (quint16)12;
    out << metaData.iterations;     // Iterationen
    out << metaData.memoryCost;     // 2GB RAM in KiB
    out << metaData.parallelism;    // Parallelism

    // 5. ChaCha20 Nonce (Tag 0x05)
    out << (quint8)0x05 << (quint16)metaData.nonce.size();
    file.write(metaData.nonce);

    // 6. Auth-Tag (Tag 0x06)
    out << (quint8)0x06 << (quint16)metaData.authTag.size();
    file.write(metaData.authTag);

    // 7. Die verschlüsselten SQLite Daten (Payload)
    file.write(encryptedSQLite);

    file.close();
}
