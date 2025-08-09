#include <QApplication>

#include "dialog_about.h"
#include "ui_dialog_about.h"

Dialog_About::Dialog_About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_About)
{
    ui->setupUi(this);

    ui->label_App_Name->setText(QCoreApplication::applicationName());
    QString Version = QCoreApplication::applicationVersion();
    ui->lineEdit_Version->setText(Version);
}

Dialog_About::~Dialog_About()
{
    delete ui;
}

void Dialog_About::on_pushButton_clicked()
{
    this->accept();
}

