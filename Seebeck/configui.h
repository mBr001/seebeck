#ifndef CONFIGUI_H
#define CONFIGUI_H

#include <QDialog>

#include "config.h"

namespace Ui {
    class ConfigUI;
}

class ConfigUI : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigUI(QWidget *parent = 0);
    ~ConfigUI();

private slots:
    void on_toolButton_clicked();

    void on_buttonBox_accepted();

private:
    Config config;
    Ui::ConfigUI *ui;
};

#endif // CONFIGUI_H
