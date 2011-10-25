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

void MainWindow::on_autoMeasAddPointsPushButton_clicked()
{
    int measFrom(ui->autoMeasFromSpinBox->value());
    int measTo(ui->autoMeasToSpinBox->value());
    int measStep(ui->autoMeasStepSpinBox->value());
    QString sampleIStr(ui->autoMeasSampleIDoubleSpinBox->text());

    int row(ui->autoMeasPointsTableWidget->rowCount());
    for (; measFrom <= measTo; measFrom += measStep) {
        ui->autoMeasPointsTableWidget->insertRow(row);
        QString measFromStr("%1");
        ui->autoMeasPointsTableWidget->setItem(row, 0, new QTableWidgetItem(measFromStr.arg(measFrom)));
        ui->autoMeasPointsTableWidget->setItem(row, 1, new QTableWidgetItem(sampleIStr));
        ++row;
    }
}

void MainWindow::on_autoMeasFromSpinBox_editingFinished()
{
    int measFrom(ui->autoMeasFromSpinBox->value());

    if (ui->autoMeasToSpinBox->value() < measFrom) {
        ui->autoMeasToSpinBox->setValue(measFrom);
    }
}

void MainWindow::on_autoMeasErasePointsToolButton_clicked()
{
    QList<QTableWidgetSelectionRange> selections(
                ui->autoMeasPointsTableWidget->selectedRanges());
    QSet<int> selectedRows;

    foreach(const QTableWidgetSelectionRange &s, selections) {
        int top(s.topRow());
        while (top <= s.bottomRow()) {
            selectedRows.insert(top);
            ++top;
        }
    }

    for (QSet<int>::const_iterator irow(selectedRows.end());
         irow != selectedRows.begin(); ) {
        --irow;
        ui->autoMeasPointsTableWidget->removeRow(*irow);
    }
}

void MainWindow::on_autoMeasToSpinBox_editingFinished()
{
    int measFrom(ui->autoMeasFromSpinBox->value());

    if (ui->autoMeasToSpinBox->value() < measFrom) {
        ui->autoMeasToSpinBox->setValue(measFrom);
    }
}

void MainWindow::on_experiment_furnaceTMeasured(int T)
{
    ui->furnaceTSpinBox->setValue(T);
}

void MainWindow::on_experimentAutoRadioButton_toggled(bool checked)
{
    if (!checked)
        return;
}

void MainWindow::on_experimentManualRadioButton_toggled(bool checked)
{
    if (!checked)
        return;

    Experiment::Params_t params;

    params.furnacePower = true;
    params.furnaceSettleTime = ui->furnaceSettleTimeSpinBox->value();
    params.furnaceSettleTStraggling = ui->furnaceSeetleTStragglingSpinBox->value();
    params.furnaceT = ui->furnaceTWantSpinBox->value();
    params.sampleI = ui->sampleIWantDoubleSpinBox->value();

    if (!experiment.start(params))
    {
        on_experiment_fatalError("Failed to start experiment", experiment.errorString());
    }
}

void MainWindow::on_experimentOffRadioButton_toggled(bool checked)
{
    if (checked)
        experiment.stop();
}

void MainWindow::on_furnaceTWantSpinBox_valueChanged(int arg1)
{
    if (!ui->experimentManualRadioButton->isChecked())
        return;

    Experiment::Params_t params(experiment.params());
    params.furnaceT = arg1;
    experiment.start(params);
}

void MainWindow::on_furnaceTWantSpinBox_editingFinished()
{
    if (!ui->experimentManualRadioButton->isChecked())
        return;

    Experiment::Params_t params(experiment.params());

    params.furnaceT = ui->furnaceTWantSpinBox->value();
    if (!experiment.start(params))
    {
        on_experiment_fatalError("Failed to start experiment", experiment.errorString());
    }
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

    Experiment::Params_t params(experiment.params());
    ui->furnaceTWantSpinBox->setValue(params.furnaceT);

    // turn experiment off -> experiment is set up at star (manual or automatic)
    ui->experimentOffRadioButton->setChecked(true);

    int Tmin, Tmax;
    if (!experiment.furnaceTRange(&Tmin, &Tmax)) {
        on_experiment_fatalError("Eurotherm operation error",
                                 "Failed to get furnace T operation range.");
        return;
    }
    ui->furnaceTWantSpinBox->setRange(Tmin, Tmax);
    ui->autoMeasFromSpinBox->setRange(Tmin, Tmax);
    ui->autoMeasToSpinBox->setRange(Tmin, Tmax);

    QWidget::show();
}

void MainWindow::startApp()
{
    configUI.show();
}
