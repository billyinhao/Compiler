#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include "helpdialog.h"
#include "errordialog.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_StartCompile_triggered();

    void on_Exit_triggered();

    void on_Help_triggered();

    void on_About_triggered();

    void on_Import_triggered();

    void on_fontComboBox_currentTextChanged(const QString &arg1);

    void on_spinBox_valueChanged(int arg1);

    void on_textEdit_textChanged();

    void on_Clear_triggered();

    void on_Export_triggered();

    void on_tokenBrowser_textChanged();

    void on_checkBox_clicked(bool checked);

    void on_FontSizeSlider_valueChanged(int value);

    void on_Restart_triggered();

private:
    Ui::MainWindow *ui;
    HelpDialog *helpdlg;
    ErrorDialog *errdlg;
    QFileDialog *filedlg;

signals:
    void help(int);
};

#endif // MAINWINDOW_H
