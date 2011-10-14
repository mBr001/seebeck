#include <QCloseEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    config(),
    configUI()
{
    ui->setupUi(this);
    QObject::connect(&configUI, SIGNAL(accepted()), this, SLOT(show()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (configUI.isHidden() && configUI.result() == QDialog::Accepted) {
        event->ignore();

        //experiment.close();
        hide();
        configUI.show();
        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::show()
{
    //if (!experiment.open())
    //    return;
    //configUI.ui->msdpPortComboBox->editText();

    QWidget::show();
}

void MainWindow::startApp()
{
    configUI.show();
}
