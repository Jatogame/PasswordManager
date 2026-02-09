#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dbHeader.h"
#include "passwordrow.h"
#include "dbManager.h"
#include <QDir>
#include <QFileDialog>
#include <QSqlDatabase>
#include <QProgressDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //update the value for the slider-value on the password generator page
    connect(ui->genpass_slider, &QSlider::valueChanged, this, [=](int value) {
        ui->genpass_slidervalue->setText(QString::number(value));
    });
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
    runTime.encryptedSQL.clear();
}

//delete DB connection before exiting Application
void MainWindow::cleanupDatabase() {
    DatabaseManager::instance().closeAndLock();
    //delete entries in the passwords-page
    while (QLayoutItem *child = ui->passwords_vertical->takeAt(0)) {
        if (QWidget *w = child->widget()) {
            w->deleteLater();
        }
        delete child;
    }
}

void MainWindow::refreshPasswords(){
    // 1) Clear existing items
    while (QLayoutItem *child = ui->passwords_vertical->takeAt(0)) {
        if (QWidget *w = child->widget()) {
            w->deleteLater();
        }
        delete child;
    }

    // 2) Get the open DB connection from your manager
    QSqlDatabase db = DatabaseManager::instance().db();
    if (!db.isOpen()) {
        return;
    }

    // 3) Query the database
    QSqlQuery query(db);
    query.prepare("SELECT id, name, url, username, notes FROM passwords ORDER BY name");

    if (!query.exec()) {
        // qDebug() << query.lastError().text();
        return;
    }

    // 4) Populate UI
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        QString url = query.value(2).toString();
        QString username = query.value(3).toString();
        QString notes = query.value(4).toString();

        auto *row = new PasswordRow(id, name, url, username, notes, this);
        ui->passwords_vertical->addWidget(row);
    }
}

//Page navigation (stacked Widget)
void MainWindow::on_sidebar_lock_clicked()
{
    //close DB connection
    DatabaseManager::instance().closeAndLock();

    //wipe the RunTime-struct
    wipeRuntimeStruct();

    //delete password-page entries
    while (QLayoutItem *child = ui->passwords_vertical->takeAt(0)) {
        if (QWidget *w = child->widget()) {
            w->deleteLater();
        }
        delete child;
    }

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
    ui->createdb_error->clear(); //clear error text

    //File Dialog to select a filename and location
    QString pathCreate = QFileDialog::getSaveFileName(
        this,
        "Create New Database",
        QDir::homePath(),
        "JaPass Vault (*.japass1)"
        );

    if (pathCreate.isEmpty())
        return;

    //add the file extension
    QFileInfo info(pathCreate);
    if (info.suffix().toLower() != "japass1") {
        pathCreate += ".japass1";
        info.setFile(pathCreate);
    }

    //checks if file with same name already exists and writes an error, if so
    if (info.exists()) {
        ui->createdb_error->setText("File with the same name already exists");
    } else{
        ui->createdb_displaypath->setText(pathCreate);
    }

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
    //check if password is empty
    if (masterPasswordStr.trimmed().isEmpty()) {
        ui->createdb_error->setText("Password empty");
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

    runTime.filePath = filePath; //save the filePath to use it in function

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
    // 1. Wipe the derived encryption key
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
    if (!masterPassword.isEmpty()) {
        sodium_memzero(masterPassword.data(), masterPassword.size());
        masterPassword.clear();
    }
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
    DatabaseManager::instance().closeAndLock();

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
    refreshPasswords();
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


void MainWindow::on_genpass_gen_clicked()
{
    QString genPassword = "";

    //get inputs
    int length = ui->genpass_slider->value();
    bool lower = ui->genpass_lower->isChecked();
    bool upper = ui->genpass_upper->isChecked();
    bool numbers = ui->genpass_numbers->isChecked();
    bool special = ui->genpass_special->isChecked();

    //generate password
    genPassword = generatePassword(length, lower, upper, numbers, special);

    ui->genpass_password->setText(genPassword); //show password
}

void MainWindow::on_passwords_create_clicked()
{
    //go to "passwordscreate-page"
    ui->stackedWidget->setCurrentIndex(6);
}


void MainWindow::on_passwordcreate_cancel_clicked()
{
    //clear inputs and show the passwords-page
    ui->passwordcreate_error->clear();
    ui->passwordcreate_name->clear();
    ui->passwordcreate_tag->clear();
    ui->passwordcreate_url->clear();
    ui->passwordcreate_username->clear();
    ui->passwordcreate_password->clear();
    ui->passwordcreate_notes->clear();
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_passwordcreate_save_clicked()
{
    ui->passwordcreate_error->clear();
    //get inputs
    QString name = ui->passwordcreate_name->text();
    QString tag = ui->passwordcreate_tag->text();
    QString url = ui->passwordcreate_url->text();
    QString username = ui->passwordcreate_username->text();
    QString password = ui->passwordcreate_password->text();
    QString notes = ui->passwordcreate_notes->text();

    //check if empty
    if(name.trimmed().isEmpty()){
        ui->passwordcreate_error->setText("Name empty");
        return;
    }


    //create new db entry and save the file
    int entrycode = DatabaseManager::instance().createentry(name, tag, url, username, password, notes);

    //save database
    saveDatabase();

    //handle SQL-error
    if (entrycode == -1){
        ui->passwordcreate_error->setText("Entry \"" + name + "\" already exists");
        return;
    }
    if (entrycode == -2){
        ui->passwordcreate_error->setText("Error: Adding new entry failed.");
        return;
    }

    //refresh passwords-list
    refreshPasswords();

    //clear inputs and show the passwords-page
    ui->passwordcreate_name->clear();
    ui->passwordcreate_tag->clear();
    ui->passwordcreate_url->clear();
    ui->passwordcreate_username->clear();
    ui->passwordcreate_password->clear();
    ui->passwordcreate_notes->clear();
    ui->stackedWidget->setCurrentIndex(1);
}

