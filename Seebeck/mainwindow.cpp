#include <QCloseEvent>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    config(),
    configUI(),
    experiment(this),
    ui(new Ui::MainWindow)
{
    experiment.setObjectName("experiment");
    ui->setupUi(this);
    QObject::connect(&configUI, SIGNAL(accepted()), this, SLOT(show()));

    Experiment::Params_t params(experiment.params());
    // TODO: set-up from config
    experiment.start(params);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::close()
{
    experiment.close();
    hide();
    configUI.show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (configUI.isHidden() && configUI.result() == QDialog::Accepted) {
        event->ignore();

        close();
        return;
    }

    QMainWindow::closeEvent(event);
}

void MainWindow::on_experiment_fatalError(const QString &errorShort, const QString &errorLong)
{
    QString title("Fatal error in experiment: ");
    QString text("%1:\n\n%2");

    text = text.arg(errorShort).arg(errorLong);
    title.append(errorShort);
    QMessageBox::critical(this, title, text);
    close();
}

void MainWindow::on_experiment_furnaceTMeasured(int T)
{
    ui->furnaceTSpinBox->setValue(T);
}

void MainWindow::show()
{
    if (!experiment.open_00(
                config.eurothermPort(),
                config.eurothermSlave(),
                config.hp34970Port(),
                config.msdpPort(),
                config.dataDir())) {
        close();
        return;
    }
    //configUI.ui->msdpPortComboBox->editText();

    QWidget::show();
}

void MainWindow::startApp()
{
    configUI.show();
}

void MainWindow::on_furnaceTWantSpinBox_valueChanged(int arg1)
{
    Experiment::Params_t params(experiment.params());
    params.furnaceT = arg1;
    experiment.start(params);
}

void MainWindow::on_experimentOffRadioButton_clicked()
{
    experiment.stop();
}

void MainWindow::on_experimentManualRadioButton_clicked()
{
    // TODO
    Experiment::Params_t params;
    params.furnaceSettleTime = ui->furnaceSettleTimeSpinBox->value();
    params.furnaceT = ui->furnaceTWantSpinBox->value();
    params.furnaceSettleTStraggling = ui->furnaceSeetleTStragglingSpinBox->value();
    params.sampleI = ui->sampleIWantDoubleSpinBox->value();
    // TODO
    if (!experiment.start(params))
    {
        ui->statusBar->showMessage("Failed to start experiment: " + experiment.errorString());
    }
}
