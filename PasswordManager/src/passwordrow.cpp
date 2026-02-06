#include "passwordrow.h"
#include "ui_passwordrow.h"

PasswordRow::PasswordRow(int id, QString name, QString url, QString username, QString notes, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PasswordRow)
    , m_id(id)
{
    ui->setupUi(this);
    ui->password_name->setText(name);
    ui->password_url->setText(url);
    ui->password_username->setText(username);
    ui->password_notes->setText(notes);
}

PasswordRow::~PasswordRow()
{
    delete ui;
}
