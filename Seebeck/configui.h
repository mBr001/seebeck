#ifndef CONFIGUI_H
#define CONFIGUI_H

#include <QDialog>

#include "config.h"
#include "../QSerialPortProbe/qserialportprobe.h"

namespace Ui {
    class ConfigUI;
}

class ConfigUI : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigUI(QWidget *parent = 0);
    ~ConfigUI();

public slots:
    virtual void accept();

private slots:
    void on_dataDirToolButton_clicked();
    void on_detectPushButton_clicked();

private:
    Config config;
    QSerialPortProbe probe;
    Ui::ConfigUI *ui;
};

#endif // CONFIGUI_H
