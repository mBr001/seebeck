#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "experiment.h"
#include "configui.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void startApp();

private:
    Ui::MainWindow *ui;
    Config config;
    ConfigUI configUI;

public slots:
    void closeEvent(QCloseEvent *event);
    void show();
};

#endif // MAINWINDOW_H
