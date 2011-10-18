#include <QFileDialog>
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

    ui->dataDirLineEdit->setText(config.dataDir());
    ui->eurothermPortComboBox->setEditText(config.eurothermPort());
    ui->eurothermSleaveSpinBox->setValue(config.eurothermSlave());
    ui->hp34970PortComboBox->setEditText(config.hp34970Port());
    ui->msdpPortComboBox->setEditText(config.msdpPort());
}

ConfigUI::~ConfigUI()
{
    delete ui;
}

void ConfigUI::on_toolButton_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(
                this, "Directory to store experiment progress logs.");
    if (dirName.isEmpty())
        return;

    ui->dataDirLineEdit->setText(dirName);
}

void ConfigUI::on_buttonBox_accepted()
{
    config.setDataDir(ui->dataDirLineEdit->text());
    config.setEurothermPort(ui->eurothermPortComboBox->currentText());
    config.setEurothermSlave(ui->eurothermSleaveSpinBox->value());
    config.setHp34970Port(ui->hp34970PortComboBox->currentText());
    config.setMsdpPort(ui->msdpPortComboBox->currentText());
}
