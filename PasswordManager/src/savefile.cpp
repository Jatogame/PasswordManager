#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include "dbHeader.h"

void saveDatabase() {

    //encrypt data
    encryptDB();

    QFile file(runTime.filePath);
    if (!file.open(QIODevice::WriteOnly)) return;

    QDataStream out(&file);
    out.setByteOrder(QDataStream::BigEndian);

    // 1. Magic ID (Tag 0x01)
    out << (quint8)0x01 << (quint16)8;
    out.writeRawData("JAPASSDB", 8);

    // 2. Version (Tag 0x02)
    out << (quint8)0x02 << (quint16)4 << (quint32)metaData.version;

    // 3. Argon2 Salt (Tag 0x03)
    out << (quint8)0x03 << (quint16)metaData.salt.size();
    out.writeRawData(metaData.salt.constData(), metaData.salt.size());

    // 4. Argon2 Params (Tag 0x04) - 12 Bytes
    out << (quint8)0x04 << (quint16)12;
    out << (quint32)metaData.iterations;     // Iterationen
    out << (quint32)metaData.memoryCost;     // 2GB RAM in KiB
    out << (quint32)metaData.parallelism;    // Parallelism

    // 5. ChaCha20 Nonce (Tag 0x05)
    out << (quint8)0x05 << (quint16)metaData.nonce.size();
    out.writeRawData(metaData.nonce.constData(), metaData.nonce.size());

    // 6. Auth-Tag (Tag 0x06)
    out << (quint8)0x06 << (quint16)metaData.authTag.size();
    out.writeRawData(metaData.authTag.constData(), metaData.authTag.size());

    // 7. encrypt SQLite Data
    out << (quint8)0x07 << (quint32)runTime.encryptedSQL.size();
    out.writeRawData(runTime.encryptedSQL.constData(), runTime.encryptedSQL.size());

    file.close();
}
