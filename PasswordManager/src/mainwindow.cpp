#include "mainwindow.h"
#include "ui_mainwindow.h"
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
}

MainWindow::~MainWindow()
{
    cleanupDatabase();
    wipeRuntimeStruct();
    delete ui;
}

//open the lock page
void MainWindow::open_locked()
{
    ui->sidebar_lock->click();
}

//delete data in struct when closing Application
void MainWindow::wipeRuntimeStruct() {
    // 1. SENSITIVE: Wipe binary keys/data with Libsodium
    // These are the most dangerous to leave in RAM
    if (!runTime.derPass.isEmpty()) {
        sodium_memzero(runTime.derPass.data(), runTime.derPass.size());
        runTime.derPass.clear();
    }

    if (!runTime.decryptedSQL.isEmpty()) {
        sodium_memzero(runTime.decryptedSQL.data(), runTime.decryptedSQL.size());
        runTime.decryptedSQL.clear();
    }

    // 2. NON-SENSITIVE
    runTime.filePath.clear();
    runTime.encryptedSQL.clear(); // Already encrypted, but good to clear
}

//delete DB connection before exiting Application
void MainWindow::cleanupDatabase() {
    // 1. Get the connection name
    QString connectionName = "internal_db";

    {
        // 2. Scope the database object
        // We put this in brackets so the 'db' object is destroyed
        // before we call removeDatabase.
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (db.isOpen()) {
            db.close();
        }
    }

    // 3. Remove the connection from Qt's global list
    QSqlDatabase::removeDatabase(connectionName);
}

//Page navigation (stacked Widget)
void MainWindow::on_sidebar_lock_clicked()
{
    //close DB connection
    void closeAndLock();
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
    //clear inputs of lock screen
    ui->lock_error->clear();
    ui->lock_displaypath->clear();
    ui->lock_masterp->clear();

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
    // 1. Wipe the derived encryption key (The most sensitive part)
    if (!runTime.derPass.isEmpty()) {
        sodium_memzero(runTime.derPass.data(), runTime.derPass.size());
        runTime.derPass.clear();
    }

    // 2. Wipe the decrypted SQL data
    if (!runTime.decryptedSQL.isEmpty()) {
        sodium_memzero(runTime.decryptedSQL.data(), runTime.decryptedSQL.size());
        runTime.decryptedSQL.clear();
    }

    // 3. Wipe the passwords
    masterPasswordStr.fill('\0');
    masterPasswordStr.clear();

    confirmPasswordStr.fill('\0');
    confirmPasswordStr.clear();

    // 4. Encrypted data and paths are less sensitive, but clearing is good practice
    runTime.encryptedSQL.clear();
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

//open decrypted file
void MainWindow::on_entermasterpassword_clicked()
{
    //make sure existing DB is closed
    void closeAndLock();

    ui->lock_error->clear(); //clear error message

    //read inputs
    QString filePath = ui->lock_displaypath->toPlainText();
    QString mpLoginStr = ui->lock_masterp->text();

    runTime.filePath = filePath;

    //check, that file path isn't empty
    if (filePath.isEmpty()) {
        ui->lock_error->setText("File Path empty");
        return;
    }

    //turn datatype to QByteArray (needed to handle it in function)
    QByteArray mpLogin = mpLoginStr.toUtf8();

    //read the file and extract the header info
    if (loadDatabase() != true){
        ui->lock_error->setText("Failed load Database");
        return;
    }

    //key derivation
    if (derivePassword(mpLogin) != true){
        ui->lock_error->setText("Failed to derive Password");
        return;
    }

    //decrypt Database
    if (decryptDB() != true){
        ui->lock_error->setText("Failed to decrypt Database");
        return;
    }

    //open passwords-page
    ui->sidebar_passwords->setChecked(true);
    ui->stackedWidget->setCurrentIndex(1);
    ui->sidebar_lock->setChecked(false);
    ui->sidebar_healthcheck->setChecked(false);
    ui->sidebar_passwordgenerator->setChecked(false);
    ui->sidebar_settings->setChecked(false);

    //delete inputs
    ui->lock_error->clear();
    ui->lock_displaypath->clear();
    ui->lock_masterp->clear();

    //securely empty
    mpLoginStr.fill('\0');
    mpLoginStr.clear();

    mpLogin.fill('\0');
    mpLogin.clear();

    sodium_memzero(runTime.decryptedSQL.data(), runTime.decryptedSQL.size());
    runTime.decryptedSQL.clear();
}

