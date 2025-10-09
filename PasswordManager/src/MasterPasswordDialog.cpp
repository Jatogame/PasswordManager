#include "MasterPasswordDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

MasterPasswordDialog::MasterPasswordDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Masterpassword");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *label = new QLabel("Enter Password:");
    passwordInput = new QLineEdit();
    passwordInput->setEchoMode(QLineEdit::Password);

    mainLayout->addWidget(label);
    mainLayout->addWidget(passwordInput);

    // Buttons
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Cancel");

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connect Signals
    connect(okButton, &QPushButton::clicked, this, &MasterPasswordDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &MasterPasswordDialog::reject);
}

QString MasterPasswordDialog::getPassword() const
{
    return passwordInput->text();
}
