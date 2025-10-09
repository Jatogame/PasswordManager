#ifndef MASTERPASSWORDDIALOG_H
#define MASTERPASSWORDDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

class MasterPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MasterPasswordDialog(QWidget *parent = nullptr);
    QString getPassword() const;

private:
    QLineEdit *passwordInput;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // MASTERPASSWORDDIALOG_H
