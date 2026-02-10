#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <qnetworkaccessmanager.h>

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
    //When opening the Application, open it in lock screen
    void open_locked();
    //delete DB connection
    void cleanupDatabase();
    //clean runTime struct
    void wipeRuntimeStruct();

    void refreshPasswords(const QString &filter);

    void refreshPasswords();

    void checkPasswordWithHIBP(const QString &password, std::function<void(int)> onDone);

    //Items for health-check-page
    struct HealthItem {
        int id;
        QString name;
        QString password;   // you need plaintext here to hash; decrypt on demand if you store encrypted
    };

    QVector<HealthItem> m_healthQueue;
    int m_healthIndex = 0;

    void runNextHealthCheck();
    void addPwnedLine(const QString &name, int count);

    //destructor
    ~MainWindow();

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

    void on_lock_selectdb_clicked();

    void on_entermasterpassword_clicked();

    void on_genpass_gen_clicked();

    void on_passwords_create_clicked();

    void on_passwordcreate_cancel_clicked();

    void on_passwordcreate_save_clicked();

    void on_passwordcreate_genpass_clicked();

    void on_passwordcreate_showpass_toggled(bool checked);

    void copyRowPassword(int id);

    void editPasswordEntry(int id);

    void on_passwordedit_delete_clicked();

    void on_passwordedit_cancel_clicked();

    void on_passwordedit_save_clicked();

    void on_passwordedit_genpass_clicked();

    void on_passwordedit_showpass_toggled(bool checked);

    void on_healthcheck_check_clicked();

private:
    QNetworkAccessManager *networkAccessManager;

    int m_currentEditId = -1;

    Ui::MainWindow *ui;

    QSqlDatabase m_db;
};
#endif // MAINWINDOW_H
