#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QFile>
#include <QTextStream>

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = 0);
    ~ErrorDialog();

private slots:
    void showerror();

    void on_OK_clicked();

private:
    Ui::ErrorDialog *ui;

};

#endif // ERRORDIALOG_H
