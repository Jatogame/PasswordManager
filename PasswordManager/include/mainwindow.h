#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dbmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //When opening the Application, open it in lock screen
    void open_locked();

private slots:
    //slots for buttons
    void on_sidebar_passwords_clicked();

    void on_sidebar_healthcheck_clicked();

    void on_sidebar_passwordgenerator_clicked();

    void on_sidebar_settings_clicked();

    void on_sidebar_lock_clicked();

    void on_newdb_clicked();

    void on_createdb_cancel_clicked();

    void on_createdb_create_clicked();

    void on_createdb_selectpath_clicked();

private:
    Ui::MainWindow *ui;
    DatabaseManager *m_dbManager;
};
#endif // MAINWINDOW_H
