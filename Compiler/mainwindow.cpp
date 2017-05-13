#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"

QLabel *statuslabel;
bool compile_enabled = true;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());
    setWindowTitle(QStringLiteral("类C语言编译器"));
    statuslabel = new QLabel(this);
    ui->statusBar->addWidget(statuslabel);
    statuslabel->setText(QStringLiteral("欢迎使用类C语言编译器！"));
    ui->StartCompile->setEnabled(false);
    ui->Export->setEnabled(false);
    ui->FontSizeSlider->setEnabled(false);
    ui->textEdit->setFont(QFont("Consolas",10));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_StartCompile_triggered()
{
    string src=ui->textEdit->toPlainText().toStdString();
    if(compiler_main(src)){
        OutList lastptr = tokenlist->next;
        while(lastptr != NULL){
            ui->tokenBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = classifylist->next;
        while(lastptr != NULL){
            ui->classifyBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = symbolist->next;
        while(lastptr != NULL){
            ui->symboBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = typelist->next;
        while(lastptr != NULL){
            ui->typeBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = ainflist->next;
        while(lastptr != NULL){
            ui->ainfBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = rinflist->next;
        while(lastptr != NULL){
            ui->rinfBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = fvlist->next;
        while(lastptr != NULL){
            ui->fvBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = pfinflist->next;
        while(lastptr != NULL){
            ui->pfinfBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = conslist->next;
        while(lastptr != NULL){
            ui->consBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = lenlist->next;
        while(lastptr != NULL){
            ui->lenBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        lastptr = quartlist->next;
        while(lastptr != NULL){
            ui->quartBrowser->append(lastptr->str);
            lastptr = lastptr->next;
        }
        statuslabel->setText(QStringLiteral("源文件编译成功！"));
    }
    else{
        statuslabel->setText(QStringLiteral("源文件编译失败！"));
        errdlg = new ErrorDialog(this);
        errdlg->exec();
    }
    DestroyList(tokenlist);
    DestroyList(classifylist);
    DestroyList(symbolist);
    DestroyList(typelist);
    DestroyList(ainflist);
    DestroyList(rinflist);
    DestroyList(fvlist);
    DestroyList(pfinflist);
    DestroyList(conslist);
    DestroyList(lenlist);
    DestroyList(quartlist);
    compile_enabled = false;
    ui->StartCompile->setEnabled(false);
}

void MainWindow::on_Exit_triggered()
{
    this->close();
}

void MainWindow::on_Help_triggered()
{
    helpdlg=new HelpDialog(this);
    connect(this,SIGNAL(help(int)),helpdlg,SLOT(selectpage(int)));
    emit help(0);
    helpdlg->exec();
}

void MainWindow::on_About_triggered()
{
    helpdlg=new HelpDialog(this);
    connect(this,SIGNAL(help(int)),helpdlg,SLOT(selectpage(int)));
    emit help(1);
    helpdlg->exec();
}

void MainWindow::on_Import_triggered()
{
    filedlg=new QFileDialog(this);
    filedlg->setAcceptMode(QFileDialog::AcceptOpen);
    filedlg->setWindowTitle(QStringLiteral("导入源文件"));
    filedlg->setNameFilter(tr("Text Documents(*.txt)"));
    if(filedlg->exec() == QDialog::Accepted) {
           QString path = filedlg->selectedFiles()[0];
           QFile file(path);
           if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
               statuslabel->setText(QStringLiteral("源文件导入成功！"));
               ui->textEdit->setPlainText(file.readAll());
           }
    }
}

void MainWindow::on_fontComboBox_currentTextChanged(const QString &arg1)
{
    ui->textEdit->setFontFamily(arg1);
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    ui->textEdit->setFontPointSize(qreal(arg1));
}

void MainWindow::on_textEdit_textChanged()
{
    if(ui->textEdit->toPlainText()=="") {
        ui->StartCompile->setEnabled(false);
    }
    else ui->StartCompile->setEnabled(compile_enabled);
}

void MainWindow::on_Export_triggered()
{
    filedlg=new QFileDialog(this);
    filedlg->setWindowTitle(QStringLiteral("导出编译结果"));
    filedlg->setFileMode(QFileDialog::Directory);
    if(filedlg->exec() == QDialog::Accepted) {
        QString path = filedlg->selectedFiles()[0];
        QFile file;
        QTextStream stream(&file);
        file.setFileName(path+QString("//TokenTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->tokenBrowser->toPlainText();
            stream << "\n\n";
            stream << ui->classifyBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//SymbolTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->symboBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//TypeTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->typeBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//ArrayTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->ainfBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//StructureTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->rinfBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//FormalParamTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->fvBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//FunctionTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->pfinfBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//ConstantTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->consBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//LengthTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->lenBrowser->toPlainText();
            file.close();
        }
        file.setFileName(path+QString("//QuaternionTable.txt"));
        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
            stream << ui->symboBrowser->toPlainText();
            file.close();
        }
        statuslabel->setText(QStringLiteral("编译结果导出成功！"));
    }
}

void MainWindow::on_Clear_triggered()
{
    ui->tokenBrowser->clear();
    ui->classifyBrowser->clear();
    ui->symboBrowser->clear();
    ui->typeBrowser->clear();
    ui->ainfBrowser->clear();
    ui->rinfBrowser->clear();
    ui->fvBrowser->clear();
    ui->pfinfBrowser->clear();
    ui->lenBrowser->clear();
    ui->consBrowser->clear();
    ui->quartBrowser->clear();
}

void MainWindow::on_tokenBrowser_textChanged()
{
    if(ui->tokenBrowser->toPlainText() == ""){
        ui->Export->setEnabled(false);
        ui->FontSizeSlider->setEnabled(false);
    }
    else {
        ui->Export->setEnabled(true);
        ui->FontSizeSlider->setEnabled(true);
    }
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    if(checked == true) ui->stackedWidget->setCurrentIndex(1);
    else ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_FontSizeSlider_valueChanged(int value)
{
    ui->tokenBrowser->setFont(QFont("宋体",qreal(value)));
    ui->classifyBrowser->setFont(QFont("宋体",qreal(value)));
    ui->symboBrowser->setFont(QFont("宋体",qreal(value)));
    ui->typeBrowser->setFont(QFont("宋体",qreal(value)));
    ui->ainfBrowser->setFont(QFont("宋体",qreal(value)));
    ui->rinfBrowser->setFont(QFont("宋体",qreal(value)));
    ui->fvBrowser->setFont(QFont("宋体",qreal(value)));
    ui->pfinfBrowser->setFont(QFont("宋体",qreal(value)));
    ui->lenBrowser->setFont(QFont("宋体",qreal(value)));
    ui->consBrowser->setFont(QFont("宋体",qreal(value)));
    ui->quartBrowser->setFont(QFont("宋体",qreal(value)));
}

void MainWindow::on_Restart_triggered()
{
    qApp->exit(773);
}
