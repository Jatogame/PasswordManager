#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

QString sha1UpperHex(const QString &password)
{
    const QByteArray bytes = password.toUtf8();
    const QByteArray digest = QCryptographicHash::hash(bytes, QCryptographicHash::Sha1);
    return QString::fromLatin1(digest.toHex()).toUpper(); // 40 hex chars
}

int parsePwnedCount(const QByteArray &body, const QString &wantedSuffixUpper)
{
    // Body lines: <SUFFIX>:<COUNT>\r\n
    const QList<QByteArray> lines = body.split('\n');
    for (const QByteArray &lineRaw : lines) {
        const QByteArray line = lineRaw.trimmed();
        if (line.isEmpty()) continue;

        const int colon = line.indexOf(':');
        if (colon <= 0) continue;

        const QString suffix = QString::fromLatin1(line.left(colon)).trimmed().toUpper();
        const int count = QString::fromLatin1(line.mid(colon + 1)).trimmed().toInt();

        if (suffix == wantedSuffixUpper) {
            return count; // if padding matched (count 0), it will return 0
        }
    }
    return 0; // not found => not pwned
}


/*
//check if password is pwned
checkPasswordWithHIBP(password, [this](int count){
    if (count > 0) {
    }
});
*/
