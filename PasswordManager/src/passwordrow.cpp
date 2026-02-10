#include "passwordrow.h"
#include "ui_passwordrow.h"

PasswordRow::PasswordRow(int id, QString name, QString url, QString username, QString notes, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PasswordRow)
    , m_id(id)
{
    ui->setupUi(this);
    ui->passshow_name->setText(name);
    ui->passshow_url->setText(url);
    ui->passshow_username->setText(username);
    ui->passshow_notes->setText(notes);

    //connect copy-password button
    connect(ui->passshow_passcopy, &QPushButton::clicked, this, [this]()  {
        emit copyPassword(m_id);
    });

    connect(ui->passshow_edit, &QPushButton::clicked, this, [this]()  {
        emit editEntry(m_id);
    });
}

PasswordRow::~PasswordRow()
{
    delete ui;
}
