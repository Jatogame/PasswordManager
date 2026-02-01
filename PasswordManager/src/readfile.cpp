#include <QSqlDatabase>
#include <QFile>
#include <QtSql/QSqlDriver>
#include "dbHeader.h"

bool loadDatabase() {
    QFile file(runTime.filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QDataStream in(&file);
    in.setByteOrder(QDataStream::BigEndian);

    // We loop until we've read the entire file
    while (!in.atEnd()) {
        quint8 tag;
        in >> tag; // Read the Tag (1 byte)

        // Read the Length
        // Tag 0x07 (Data) uses quint32, all others use quint16
        quint32 length;
        if (tag == 0x07) {
            in >> length;
        } else {
            quint16 tempLen;
            in >> tempLen;
            length = tempLen;
        }

        // Standard safety check: don't try to read more than exists
        if (in.status() != QDataStream::Ok) break;

        switch (tag) {
        case 0x01: { // Magic ID
            QByteArray magic(length, 0);
            in.readRawData(magic.data(), length);
            if (magic != "JAPASSDB") {
                file.close();
                return false; // Wrong file type!
            }
            break;
        }
        case 0x02: // Version
            in >> metaData.version;
            break;

        case 0x03: // Salt
            metaData.salt.resize(length);
            in.readRawData(metaData.salt.data(), length);
            break;

        case 0x04: // Argon2 Params (Iterations, Memory, Parallelism)
            in >> metaData.iterations;
            in >> metaData.memoryCost;
            in >> metaData.parallelism;
            break;

        case 0x05: // Nonce
            metaData.nonce.resize(length);
            in.readRawData(metaData.nonce.data(), length);
            break;

        case 0x06: // Auth-Tag
            metaData.authTag.resize(length);
            in.readRawData(metaData.authTag.data(), length);
            break;

        case 0x07: // Encrypted SQL Data
            runTime.encryptedSQL.resize(length);
            in.readRawData(runTime.encryptedSQL.data(), length);
            break;

        default:
            // Skip unknown tags (Forward Compatibility)
            in.skipRawData(length);
            break;
        }
    }

    file.close();
    return (in.status() == QDataStream::Ok);
}
