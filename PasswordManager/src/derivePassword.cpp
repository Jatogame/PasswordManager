#include <QString>
#include <QFileDialog>
#include <QSqlDatabase>
#include "dbHeader.h"

void derivePassword(QByteArray masterPassword){
    RunTimeData runTime;
    //use argon2id to derive password
    QByteArray derPassword;
    //save derived password
    runTime.derPass = derPassword;
};
