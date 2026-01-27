#ifndef DBHEADER_H
#define DBHEADER_H
#include <QObject>

struct DbHeader {
    uint16_t version = 1;
    QByteArray salt;      // 16 Bytes
    uint32_t iterations = 20;
    uint32_t memoryCost = 2097152; // 2GB
    uint32_t parallelism = 1;
    QByteArray nonce;     // 12 Bytes
    QByteArray authTag;   // 16 Bytes
};

#endif // DBHEADER_H
