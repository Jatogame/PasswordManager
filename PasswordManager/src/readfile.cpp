#include <QSqlDatabase>
#include <QFile>
#include <QtSql/QSqlDriver>
#include "dbHeader.h"

bool loadDatabase() {
    QFile file(runTime.filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QDataStream in(&file);
    in.setByteOrder(QDataStream::BigEndian);

    while (!in.atEnd()) {
        quint8 tag;
        in >> tag; // Read 1-byte Tag

        if (in.status() != QDataStream::Ok) break;

        switch (tag) {
        case 0x01: { // Magic ID
            quint16 len; in >> len;
            QByteArray magic(len, 0);
            in.readRawData(magic.data(), len);
            if (magic != "JAPASSDB") return false;
            break;
        }
        case 0x02: { // Version
            quint16 len; in >> len;
            in >> metaData.version;
            break;
        }
        case 0x03: { // Salt
            quint16 len; in >> len;
            metaData.salt.resize(len);
            in.readRawData(metaData.salt.data(), len);
            break;
        }
        case 0x04: { // Argon2 Params
            quint16 len; in >> len;
            in >> metaData.iterations >> metaData.memoryCost >> metaData.parallelism;
            break;
        }
        case 0x05: { // Nonce
            quint16 len; in >> len;
            metaData.nonce.resize(len);
            in.readRawData(metaData.nonce.data(), len);
            break;
        }
        case 0x06: { // Encrypted Data
            quint32 len; in >> len; // Note: quint32 for the large data
            runTime.encryptedSQL.resize(len);
            in.readRawData(runTime.encryptedSQL.data(), len);
            break;
        }
        default: {
            // Unknown tag? Read the next 2 bytes as length and skip it.
            // This makes your file format "Forward Compatible"
            quint16 unknownLen; in >> unknownLen;
            in.skipRawData(unknownLen);
            break;
        }
        }
    }

    file.close();
    return true;
}
