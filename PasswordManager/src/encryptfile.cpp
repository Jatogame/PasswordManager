#include "dbHeader.h"

void encryptDB(){
    //serialize the DB
    QByteArray rawData = serializeDatabase();

    // 1. Generate a 12-byte Nonce (IETF standard)
    metaData.nonce.resize(crypto_aead_chacha20poly1305_ietf_NPUBBYTES);
    randombytes_buf(metaData.nonce.data(), metaData.nonce.size());

    // 2. Prepare output buffer (Plaintext + Tag)
    runTime.encryptedSQL.resize(rawData.size() + crypto_aead_chacha20poly1305_ietf_ABYTES);
    unsigned long long ciphertext_len;

    // 3. Encrypt
    crypto_aead_chacha20poly1305_ietf_encrypt(
        reinterpret_cast<unsigned char*>(runTime.encryptedSQL.data()),
        &ciphertext_len,
        reinterpret_cast<const unsigned char*>(rawData.constData()),
        rawData.size(),
        nullptr, 0, // No "Additional Data" used here
        nullptr,    // No secret nonce
        reinterpret_cast<const unsigned char*>(metaData.nonce.constData()),
        reinterpret_cast<const unsigned char*>(runTime.derPass.constData())
        );

    // Clean up the raw data from RAM immediately
    sodium_memzero(rawData.data(), rawData.size());
};
