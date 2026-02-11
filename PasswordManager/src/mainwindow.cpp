#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dbHeader.h"
#include "passwordrow.h"
#include "dbManager.h"
#include <QDir>
#include <QClipboard>
#include <QFileDialog>
#include <QSqlDatabase>
#include <QProgressDialog>
#include <QTimer>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkAccessManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    //update the value for the slider-value on the password generator page
    connect(ui->genpass_slider, &QSlider::valueChanged, this, [=](int value) {
        ui->genpass_slidervalue->setText(QString::number(value));
    });

    //searchbar auto-refresh password-page
    connect(ui->passwords_search, &QLineEdit::textChanged,
            this, [this](const QString &text){
                refreshPasswords(text);
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
void MainWindow::wipeRuntimeStruct()
{
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
void MainWindow::cleanupDatabase()
{
    DatabaseManager::instance().closeAndLock();

    //clear clipboard
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->clear();
    clipboard->clear(QClipboard::Clipboard);      // Ctrl+C clipboard
    clipboard->clear(QClipboard::Selection);      // Middle-click selection (Linux)

    //delete entries in the passwords-page
    while (QLayoutItem *child = ui->passwords_vertical->takeAt(0)) {
        if (QWidget *w = child->widget()) {
            w->deleteLater();
        }
        delete child;
    }

    //Clear scroll area of health-check-page
    auto *layout = ui->scrollAreaWidgetContents_2->layout();

    while (QLayoutItem *child = layout->takeAt(0)) {
        if (QWidget *w = child->widget()){
            w->deleteLater();
        }
        delete child;
    }

    ui->passwords_search->clear();//clear search-field
}

//refresh passwordlist without search-filter
void MainWindow::refreshPasswords()
{
    refreshPasswords(ui->passwords_search ? ui->passwords_search->text() : QString());
}

//refresh passwordlist with search-filter
void MainWindow::refreshPasswords(const QString &filter)
{
    //Clear existing items
    while (QLayoutItem *child = ui->passwords_vertical->takeAt(0)) {
        if (QWidget *w = child->widget()) {
            w->deleteLater();
        }
        delete child;
    }

    //Get the open DB connection from the manager
    QSqlDatabase db = DatabaseManager::instance().db();
    if (!db.isOpen()) {
        return;
    }

    //Query the database
    QSqlQuery query(db);

    const QString f = filter.trimmed();

    //apply filter if not empty
    if (f.isEmpty()) {
        query.prepare("SELECT id, name, url, username, notes "
                      "FROM passwords ORDER BY name COLLATE NOCASE");
    } else {
        // Search multiple columns; use LIKE with bound values
        query.prepare("SELECT id, name, url, username, notes "
                      "FROM passwords "
                      "WHERE name LIKE :q OR url LIKE :q OR username LIKE :q OR notes LIKE :q "
                      "ORDER BY name COLLATE NOCASE");
        query.bindValue(":q", "%" + f + "%");
    }

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

        auto *row = new PasswordRow(id, name, url, username, notes, ui->scrollAreaWidgetContents);

        //copy-password button
        connect(row, &PasswordRow::copyPassword, this, &MainWindow::copyRowPassword);

        //edit button
        connect(row, &PasswordRow::editEntry, this, &MainWindow::editPasswordEntry);

        ui->passwords_vertical->addWidget(row);
    }
    ui->passwords_vertical->addStretch(1);
}

void MainWindow::copyRowPassword(int id)
{
    QSqlDatabase db = DatabaseManager::instance().db();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);

    query.prepare("SELECT password FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        //error
        return;
    }

    const QString password = query.value(0).toString();

    QClipboard *cb = QGuiApplication::clipboard();
    cb->setText(password, QClipboard::Clipboard);
}

void MainWindow::editPasswordEntry(int id)
{
    //go to passwordedit-page
    ui->stackedWidget->setCurrentIndex(7);

    //safe the id, so the page doesn't forget it's id
    m_currentEditId = id;

    //Select data from DB and paste it
    QSqlDatabase db = DatabaseManager::instance().db();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);

    query.prepare("SELECT name, tag, url, username, password, notes FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        //error
        return;
    }
    QString name = query.value(0).toString();
    QString tag = query.value(1).toString();
    QString url = query.value(2).toString();
    QString username = query.value(3).toString();
    QString password = query.value(4).toString();
    QString notes = query.value(5).toString();

    //paste the data into the ui
    ui->passwordedit_name->setText(name);
    ui->passwordedit_tag->setText(tag);
    ui->passwordedit_url->setText(url);
    ui->passwordedit_username->setText(username);
    ui->passwordedit_password->setText(password);
    ui->passwordedit_notes->setText(notes);
}

//Page navigation (stacked Widget)
void MainWindow::on_sidebar_lock_clicked()
{
    //clear DB and clear passwords-page
    cleanupDatabase();

    //wipe the RunTime-struct
    wipeRuntimeStruct();

    //reset password-generator page
    ui->genpass_password->clear();
    ui->genpass_lower->setChecked(genPassword.lower);
    ui->genpass_upper->setChecked(genPassword.upper);
    ui->genpass_numbers->setChecked(genPassword.numbers);
    ui->genpass_special->setChecked(genPassword.special);
    ui->genpass_slider->setValue(genPassword.length);

    //set UI elements
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


void MainWindow::on_passwordcreate_genpass_clicked()
{
    //get data for generating password
    int length = genPassword.length;
    bool lower = genPassword.lower;
    bool upper = genPassword.upper;
    bool numbers = genPassword.numbers;
    bool special = genPassword.special;
    QString genPassword = generatePassword(length, lower, upper, numbers, special);
    ui->passwordcreate_password->setText(genPassword);
}


void MainWindow::on_passwordcreate_showpass_toggled(bool checked)
{
    ui->passwordcreate_password->setEchoMode(
        checked ? QLineEdit::Normal : QLineEdit::Password
        );
}

void MainWindow::on_passwordedit_delete_clicked()
{
    if (m_currentEditId < 0)
        return;

    if (QMessageBox::question(this, "Delete entry",
                              "Really delete this entry? This cannot be undone.")
        != QMessageBox::Yes)
        return;

    QSqlDatabase db = DatabaseManager::instance().db();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);

    query.prepare("DELETE FROM passwords WHERE id = :id");
    query.bindValue(":id", m_currentEditId);

    if (!query.exec()) {
        QMessageBox::warning(this, "Error", "Failed to delete entry.");
        return;
    }

    //overwrite current id
    m_currentEditId = -1;

    //save database
    saveDatabase();

    //load password-list new
    refreshPasswords();

    //go back to passwords-page
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_passwordedit_cancel_clicked()
{
    //clear inputs and show the passwords-page
    ui->passwordedit_error->clear();
    ui->passwordedit_name->clear();
    ui->passwordedit_tag->clear();
    ui->passwordedit_url->clear();
    ui->passwordedit_username->clear();
    ui->passwordedit_password->clear();
    ui->passwordedit_notes->clear();
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_passwordedit_save_clicked()
{
    ui->passwordcreate_error->clear();
    //get inputs
    QString name = ui->passwordedit_name->text();
    QString tag = ui->passwordedit_tag->text();
    QString url = ui->passwordedit_url->text();
    QString username = ui->passwordedit_username->text();
    QString password = ui->passwordedit_password->text();
    QString notes = ui->passwordedit_notes->text();

    //check if DB is open
    QSqlDatabase db = DatabaseManager::instance().db();
    if (!db.isOpen()) {
        ui->passwordedit_error->setText("No DB open");
        return;
    }

    //check if empty
    if(name.trimmed().isEmpty()){
        ui->passwordedit_error->setText("Name empty");
        return;
    }

    //MessageBox saying that the changes cannot be undone
    if (QMessageBox::question(this, "Save entry",
                              "Really save this entry? This cannot be undone.")
        != QMessageBox::Yes)
        return;

    //create new db entry and save the file
    int entrycode = DatabaseManager::instance().editentry(m_currentEditId, name, tag, url, username, password, notes);

    //save database
    saveDatabase();

    //handle SQL-error
    if (entrycode == -1){
        ui->passwordedit_error->setText("Entry \"" + name + "\" already exists");
        return;
    }
    if (entrycode == -2){
        ui->passwordedit_error->setText("Error: Adding new entry failed.");
        return;
    }

    //refresh passwords-list
    refreshPasswords();

    //clear inputs and show the passwords-page
    ui->passwordedit_name->clear();
    ui->passwordedit_tag->clear();
    ui->passwordedit_url->clear();
    ui->passwordedit_username->clear();
    ui->passwordedit_password->clear();
    ui->passwordedit_notes->clear();
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::on_passwordedit_genpass_clicked()
{
    //get data for generating password
    int length = genPassword.length;
    bool lower = genPassword.lower;
    bool upper = genPassword.upper;
    bool numbers = genPassword.numbers;
    bool special = genPassword.special;
    QString genPassword = generatePassword(length, lower, upper, numbers, special);
    ui->passwordedit_password->setText(genPassword);
}


void MainWindow::on_passwordedit_showpass_toggled(bool checked)
{
    ui->passwordedit_password->setEchoMode(
        checked ? QLineEdit::Normal : QLineEdit::Password
        );
}

void MainWindow::checkPasswordWithHIBP(const QString &password, std::function<void(int)> onDone)
{
    const QString full = sha1UpperHex(password);
    const QString prefix = full.left(5);
    const QString suffix = full.mid(5);

    QNetworkRequest req(QUrl("https://api.pwnedpasswords.com/range/" + prefix));
    req.setRawHeader("Add-Padding", "true");
    req.setRawHeader("User-Agent", "JaPass/0.1");

    auto *reply = networkAccessManager->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, suffix, onDone]() {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();

        int count = 0;
        if (reply->error() == QNetworkReply::NoError && status == 200) {
            count = parsePwnedCount(body, suffix);
        }

        reply->deleteLater();
        onDone(count);  // <- deliver result here
    });
}


void MainWindow::on_healthcheck_check_clicked()
{
    //Clear previous results in scroll area
    auto *layout = ui->scrollAreaWidgetContents_2->layout();

    //clear Layout
    while (QLayoutItem *child = layout->takeAt(0)) {
        if (QWidget *w = child->widget()) w->deleteLater();
        delete child;
    }

    //Creates a fresh status label that stays at the top
    QLabel *statusLabel = new QLabel("Initializing...", ui->scrollAreaWidgetContents_2);
    statusLabel->setStyleSheet("font-weight: bold; color: white;"); //make it look different
    layout->addWidget(statusLabel);

    //Load entries from DB
    QSqlDatabase db = DatabaseManager::instance().db();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery q(db);
    q.prepare("SELECT id, name, password FROM passwords ORDER BY name COLLATE NOCASE");

    if (!q.exec()) {
        return;
    }

    m_healthQueue.clear();
    while (q.next()) {
        HealthItem item;
        item.id = q.value(0).toInt();
        item.name = q.value(1).toString();

        item.password = q.value(2).toString();

        //Skip empty passwords
        if (!item.password.isEmpty())
            m_healthQueue.push_back(item);
    }

    m_healthIndex = 0;

    //Start sequential checking
    runNextHealthCheck();
}

void MainWindow::runNextHealthCheck()
{
    //find the status label (first widget)
    auto *layout = ui->scrollAreaWidgetContents_2->layout();
    QLabel *statusLabel = nullptr;
    if (layout && layout->count() > 0) {
        statusLabel = qobject_cast<QLabel*>(layout->itemAt(0)->widget());
    }

    if (m_healthIndex >= m_healthQueue.size()) {
        if (statusLabel) statusLabel->setText("Health check finished.");
        return;
    }

    const auto item = m_healthQueue[m_healthIndex++];
    if (statusLabel) {
        statusLabel->setText(QString("Checking %1 (%2/%3)…")
                                 .arg(item.name)
                                 .arg(m_healthIndex)
                                 .arg(m_healthQueue.size()));
    }

    checkPasswordWithHIBP(item.password, [this, item](int count) {
        if (count > 0) {
            addPwnedLine(item.name, count);
        }
        runNextHealthCheck(); //continue with next entry
    });
}

void MainWindow::addPwnedLine(const QString &name, int count)
{
    auto *layout = ui->scrollAreaWidgetContents_2->layout();

    //Add label line
    auto *line = new QLabel(QString("⚠ %1 (pwned %2 times)").arg(name).arg(count),
                            ui->scrollAreaWidgetContents_2);
    line->setWordWrap(true);
    layout->addWidget(line);
}
