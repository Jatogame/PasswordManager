#ifndef PASSWORDROW_H
#define PASSWORDROW_H

#include <QWidget>

namespace Ui {
class PasswordRow;
}

class PasswordRow : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordRow(int id, QString name, QString url, QString username, QString notes, QWidget *parent = nullptr);
    ~PasswordRow();

signals:
    void copyPassword(int id);
    void editEntry(int id);

private:
    Ui::PasswordRow *ui;
    int m_id;
};

#endif // PASSWORDROW_H
