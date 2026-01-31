#include <QString>
#include <QFileDialog>
#include <QSqlDatabase>
#include "dbHeader.h"
#include <sodium.h>

bool createDerPassword(QByteArray masterPassword){

    //use argon2id to derive password
    QByteArray derPassword (32, 0); //32 Bytes for ChaCha20

    //calculate salt
    QByteArray salt(crypto_pwhash_SALTBYTES, 0);
    randombytes_buf(salt.data(), salt.size());
    metaData.salt = salt;   //save salt in the struct DbHeader

    //calculate Argon2id
    int result = crypto_pwhash(
        reinterpret_cast<unsigned char*>(derPassword.data()),
        derPassword.size(),
        masterPassword.constData(),
        masterPassword.size(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        metaData.iterations,
        metaData.memoryCost,
        crypto_pwhash_ALG_ARGON2ID13
        );

    if(result != 0){
        return false;
    }

    //save derived password
    runTime.derPass = derPassword;
    return true;
};
