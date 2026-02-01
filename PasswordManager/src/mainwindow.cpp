#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dbmanager.h"
#include "dbHeader.h"
#include <QDir>
#include <QFileDialog>
#include <QSqlDatabase>
#include <QProgressDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //create instance for the database manager
    m_dbManager = new DatabaseManager(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//open the lock page
void MainWindow::open_locked()
{
    ui->sidebar_lock->click();
}

//Page navigation (stacked Widget)
void MainWindow::on_sidebar_lock_clicked()
{
    ui->sidebar_lock->setChecked(true);

    ui->stackedWidget->setCurrentIndex(0);

    ui->sidebar_passwords->setChecked(false);
    ui->sidebar_healthcheck->setChecked(false);
    ui->sidebar_passwordgenerator->setChecked(false);
    ui->sidebar_settings->setChecked(false);
}

void MainWindow::on_sidebar_passwords_clicked()
{
    ui->sidebar_passwords->setChecked(true);

    ui->stackedWidget->setCurrentIndex(1);

    ui->sidebar_lock->setChecked(false);
    ui->sidebar_healthcheck->setChecked(false);
    ui->sidebar_passwordgenerator->setChecked(false);
    ui->sidebar_settings->setChecked(false);
}

void MainWindow::on_sidebar_healthcheck_clicked()
{
    ui->sidebar_healthcheck->setChecked(true);

    ui->stackedWidget->setCurrentIndex(2);

    ui->sidebar_passwords->setChecked(false);
    ui->sidebar_lock->setChecked(false);
    ui->sidebar_passwordgenerator->setChecked(false);
    ui->sidebar_settings->setChecked(false);
}

void MainWindow::on_sidebar_passwordgenerator_clicked()
{
    ui->sidebar_passwordgenerator->setChecked(true);

    ui->stackedWidget->setCurrentIndex(3);

    ui->sidebar_passwords->setChecked(false);
    ui->sidebar_healthcheck->setChecked(false);
    ui->sidebar_lock->setChecked(false);
    ui->sidebar_settings->setChecked(false);
}

void MainWindow::on_sidebar_settings_clicked()
{
    ui->sidebar_settings->setChecked(true);

    ui->stackedWidget->setCurrentIndex(4);

    ui->sidebar_passwords->setChecked(false);
    ui->sidebar_healthcheck->setChecked(false);
    ui->sidebar_passwordgenerator->setChecked(false);
    ui->sidebar_lock->setChecked(false);
}

//Open page to create new database
void MainWindow::on_newdb_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}

//display the selected path for the new database
void MainWindow::on_createdb_selectpath_clicked()
{
    QString pathCreate = QFileDialog::getSaveFileName(
        this,
        "Create New Database",
        QDir::homePath(),
        "JaPass Vault (*.japass1)"
        );

    if (pathCreate.isEmpty())
        return;

    QFileInfo info(pathCreate);
    if (info.suffix().toLower() != "japass1") {
        pathCreate += ".japass1";
    }

    ui->createdb_displaypath->setText(pathCreate);
}

//Cancel the creation of a new database
void MainWindow::on_createdb_cancel_clicked()
{
    //go to Lock page
    ui->stackedWidget->setCurrentIndex(0);
    //clear the inputs
    ui->createdb_displaypath->clear();
    ui->createdb_mp1->clear();
    ui->createdb_mp2->clear();
    ui->createdb_error->clear();
}

//validate the input and create a new database
void MainWindow::on_createdb_create_clicked()
{
    //clear the error text
    ui->createdb_error->clear();

    //get inputs
    QString filePath = ui->createdb_displaypath->toPlainText();
    QString masterPasswordStr = ui->createdb_mp1->text();
    QString confirmPasswordStr = ui->createdb_mp2->text();

    //check, that file path isn't empty
    if (filePath.isEmpty()) {
        ui->createdb_error->setText("Path is empty");
        return;
    }
    //check if passwords match
    if (masterPasswordStr != confirmPasswordStr) {
        ui->createdb_error->setText("Passwords don't match");
        return;
    }

    //turn datatype to QByteArray (needed to handle it in function)
    QByteArray masterPassword = masterPasswordStr.toUtf8();

    //set meta data for DB creation
    metaData.version = createMetaData.version;
    metaData.iterations = createMetaData.iterations;
    metaData.memoryCost = createMetaData.memoryCost;
    metaData.parallelism = createMetaData.parallelism;

    //key derivation
    if (createDerPassword(masterPassword) != true){
        ui->createdb_error->setText("Error: Key Derivation");
        return;
    }

    //save the filePath to use it in function
    masterPassword = "";
    runTime.filePath = filePath;

    if (createFile()==true){
        //go to Lock page
        ui->stackedWidget->setCurrentIndex(0);
        //clear the inputs
        ui->createdb_displaypath->clear();
        ui->createdb_mp1->clear();
        ui->createdb_mp2->clear();
        ui->createdb_error->clear();
        } else{
        ui->createdb_error->setText("Failed to create Database");
    }
    //delete saved inputs
    masterPassword = "";
    masterPasswordStr = "";
    confirmPasswordStr = "";
    runTime.derPass = "";
    runTime.filePath = "";
}

//select path for unlocking
void MainWindow::on_lock_selectdb_clicked()
{
    QString pathSelect = QFileDialog::getOpenFileName(
        this,
        "Select Database",
        QDir::homePath(),
        "JaPass Vault (*.japass1)"
        );

    if (pathSelect.isEmpty())
        return;

    ui->lock_displaypath->setText(pathSelect);
}

