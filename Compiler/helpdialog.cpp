#include "helpdialog.h"
#include "ui_helpdialog.h"

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);
}

HelpDialog::~HelpDialog()
{
    delete ui;
}

//0:help 1:about
void HelpDialog::selectpage(int i)
{
    ui->page->setCurrentIndex(i);
}

void HelpDialog::on_OK_clicked()
{
    this->close();
}
