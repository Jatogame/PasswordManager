#include "dbHeader.h"
#include <QRandomGenerator>

QString generatePassword(int length, bool useLower, bool useUpper, bool useNumbers, bool useSpecial){
    // 1. Constrain length to your maximum
    length = std::clamp(length, 1, 50);

    QString charPool = "";
    if (useLower)   charPool += "abcdefghijklmnopqrstuvwxyz";
    if (useUpper)   charPool += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (useNumbers) charPool += "0123456789";
    if (useSpecial) charPool += "~`!@#$%^&*()-_+={}[]|\\;:<>,./?";

    // Fallback if no categories are selected
    if (charPool.isEmpty()) return QString();

    QString password;
    password.reserve(length);
    int index;

    for (int i = 0; i < length; ++i) {
        // Securely pick a random index from the pool
        index = QRandomGenerator::global()->bounded(static_cast<quint32>(charPool.length()));
        password.append(charPool.at(index));
    }

    return password;
}
