#include "errordialog.h"
#include "ui_errordialog.h"

ErrorDialog::ErrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
    showerror();
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::showerror(){
    QFile file("data/log.dat");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&file);
        ui->textBrowser->setPlainText(stream.readAll());
        file.close();
    }
}

void ErrorDialog::on_OK_clicked()
{
    this->close();
}
