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
    void on_experiment_furnaceTMeasured(int T);
    void on_experiment_sampleHeatingUIMeasured(double I, double U);
    void on_experimentAutoRadioButton_toggled(bool checked);
    void on_experimentManualRadioButton_toggled(bool checked);
    void on_experimentOffRadioButton_toggled(bool checked);

    void on_manualApplyFurnacePushButton_clicked();
    void on_manualApplySamplePushButton_clicked();
};

#endif // MAINWINDOW_H
