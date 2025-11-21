#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

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

