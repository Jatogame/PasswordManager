#include "mainwindow.h"
#include "MasterPasswordDialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MasterPasswordDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0; // User clicked Cancel
    }

    QString masterPassword = loginDialog.getPassword();
    qDebug() << "Masterpassword:" << masterPassword; // for testing

    MainWindow w;
    w.show();
    return a.exec();
}
