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

protected:
    void close();

private:
    Config config;
    ConfigUI configUI;
    Experiment experiment;
    Ui::MainWindow *ui;

public slots:
    void closeEvent(QCloseEvent *event);
    void show();

private slots:
    void on_autoMeasFromSpinBox_editingFinished();
    void on_autoMeasToSpinBox_editingFinished();
    void on_experiment_fatalError(const QString &errorShort, const QString &errorLong);
    void on_experiment_furnaceTMeasured(int T, double Tstraggling);
    void on_experiment_sampleHeatingUIMeasured(double I, double U);
    void on_experiment_sampleTMeasured(double T1, double T2, double T3, double T4);
    void on_experiment_sampleUMeasured(double U12, double U23, double U34, double U41);
    void on_experimentOffRadioButton_toggled(bool checked);
    void on_manualApplyPushButton_clicked();
    void on_manualStartPushButton_clicked();
    void on_sampleSPushButton_clicked();
    void on_sampleL1DoubleSpinBox_editingFinished();
    void on_sampleL2DoubleSpinBox_editingFinished();
    void on_sampleL3DoubleSpinBox_editingFinished();
};

#endif // MAINWINDOW_H
