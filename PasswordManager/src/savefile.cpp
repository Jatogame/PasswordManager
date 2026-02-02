#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include "dbHeader.h"

void saveDatabase() {
    encryptDB(); // This now produces one QByteArray containing [Ciphertext + Tag]

    QFile file(runTime.filePath);
    if (!file.open(QIODevice::WriteOnly)) return;

    QDataStream out(&file);
    out.setByteOrder(QDataStream::BigEndian);

    // 1. Magic ID
    out << (quint8)0x01 << (quint16)8;
    out.writeRawData("JAPASSDB", 8);

    // 2. Version
    out << (quint8)0x02 << (quint16)4 << (quint32)metaData.version;

    // 3. Salt
    out << (quint8)0x03 << (quint16)metaData.salt.size();
    out.writeRawData(metaData.salt.constData(), metaData.salt.size());

    // 4. Argon2 Params
    out << (quint8)0x04 << (quint16)12;
    out << (quint32)metaData.iterations << (quint32)metaData.memoryCost << (quint32)metaData.parallelism;

    // 5. Nonce (12 bytes for ChaCha20-IETF)
    out << (quint8)0x05 << (quint16)metaData.nonce.size();
    out.writeRawData(metaData.nonce.constData(), metaData.nonce.size());

    // 6. Encrypted SQL Data (Tag 0x06)
    // Note: runTime.encryptedSQL already contains the 16-byte Auth-Tag at the end
    out << (quint8)0x06 << (quint32)runTime.encryptedSQL.size();
    out.writeRawData(runTime.encryptedSQL.constData(), runTime.encryptedSQL.size());

    file.close();
}
