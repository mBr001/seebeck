#include <QFileDialog>
#include <QMessageBox>
#include "../QSCPIDev/qserial.h"
#include "configui.h"
#include "ui_configui.h"

ConfigUI::ConfigUI(QWidget *parent) :
    QDialog(parent),
    config(),
    ui(new Ui::ConfigUI)
{
    ui->setupUi(this);
    QStringList ports = QSerial::list();
    ui->eurothermPortComboBox->addItems(ports);
    ui->hp34970PortComboBox->addItems(ports);
    ui->msdpPortComboBox->addItems(ports);
    ui->ps6220PortComboBox->addItems(ports);

    ui->dataDirLineEdit->setText(config.dataDir());
    ui->eurothermPortComboBox->setEditText(config.eurothermPort());
    ui->eurothermSleaveSpinBox->setValue(config.eurothermSlave());
    ui->hp34970PortComboBox->setEditText(config.hp34970Port());
    ui->msdpPortComboBox->setEditText(config.msdpPort());
    ui->ps6220PortComboBox->setEditText(config.ps6220Port());

    ui->devicesTableWidget->setVisible(false);
    QSize s(size());
    s.setHeight(0);
    resize(s);
}

ConfigUI::~ConfigUI()
{
    delete ui;
}

void ConfigUI::accept()
{
    QDir dataDir(ui->dataDirLineEdit->text());

    if (!dataDir.exists()) {
        QString msg("Directory: \"" + dataDir.path() +
                    "\"\ndoes not exists.\nCreate this directory?");
        if (QMessageBox::Yes != QMessageBox::question(
                    this, "Create data storage directory?", msg,
                    QMessageBox::Yes | QMessageBox::No)) {
            return;
        }
        if (!dataDir.mkpath(".")) {
            msg = "Failed to create directory: \"" + dataDir.path() + "\"";
            QMessageBox::critical(this, "Failed to create directory.", msg);
            return;
        }
    }

    config.setDataDir(dataDir.path());
    config.setEurothermPort(ui->eurothermPortComboBox->currentText());
    config.setEurothermSlave(ui->eurothermSleaveSpinBox->value());
    config.setHp34970Port(ui->hp34970PortComboBox->currentText());
    config.setMsdpPort(ui->msdpPortComboBox->currentText());
    config.setPs6220Port(ui->ps6220PortComboBox->currentText());

    QDialog::accept();
}

void ConfigUI::on_dataDirToolButton_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(
                this, "Directory to store experiment progress logs.");
    if (dirName.isEmpty())
        return;

    ui->dataDirLineEdit->setText(dirName);
}

void ConfigUI::on_detectPushButton_clicked()
{
    ui->detectPushButton->setVisible(false);
    ui->devicesTableWidget->setVisible(true);

    on_detectPushButton_clicked();
}

void ConfigUI::on_devicesRefreshToolButton_clicked()
{
    ui->devicesTableWidget->setRowCount(0);
     const QSerialPortProbe::DeviceList &devices = probe.list();
     ui->devicesTableWidget->setRowCount(devices.size());

     int row(0);
     for (QSerialPortProbe::DeviceList::ConstIterator i(devices.begin());
                                                        i != devices.end();
                                                        ++i) {
         QTableWidgetItem *item;

         item = new QTableWidgetItem(i->port());
         ui->devicesTableWidget->setItem(row, 0, item);

         item = new QTableWidgetItem(i->protocolString());
         ui->devicesTableWidget->setItem(row, 1, item);

         item = new QTableWidgetItem(i->deviceName());
         ui->devicesTableWidget->setItem(row, 2, item);
         ++row;
     }
}
