#include "config.h"

const char Config::str_data_dir[] = "data dir";
const char Config::str_eurotherm_port[] = "eurother port";
const char Config::str_eurotherm_slave[] = "eurother slave";
const char Config::str_hp34970_port[] = "hp34970 port";
const char Config::str_msdp_port[] = "msdp port";
const char Config::str_ps6220_port[] = "ps 6220 port";
const char Config::str_sample_l1[] = "sample l1";
const char Config::str_sample_l2[] = "sample l2";
const char Config::str_sample_l3[] = "sample l3";
const char Config::str_sample_S[] = "sample S";
const char Config::str_sample_h[] = "sample height";
const char Config::str_sample_w[] = "sample width";
const char Config::str_resistivity_i[] = "resistivity i";
const char Config::str_experimentator[] = "experimentator";


Config::Config()
{
}

QString Config::dataDir()
{
    return settings.value(str_data_dir).toString();
}

QString Config::eurothermPort()
{
    return settings.value(str_eurotherm_port).toString();
}

int Config::eurothermSlave()
{
    return settings.value(str_eurotherm_slave).toInt();
}

QString Config::experimentator()
{
    return settings.value(str_experimentator).toString();
}

QString Config::hp34970Port()
{
    return settings.value(str_hp34970_port).toString();
}

QString Config::msdpPort()
{
    return settings.value(str_msdp_port).toString();
}

QString Config::ps6220Port()
{
    return settings.value(str_ps6220_port).toString();
}

double Config::resistivity_I()
{
    return settings.value(str_resistivity_i).toDouble();
}

double Config::sample_h()
{
    return settings.value(str_sample_h).toDouble();
}

double Config::sample_l1()
{
    return settings.value(str_sample_l1).toDouble();
}

double Config::sample_l2()
{
    return settings.value(str_sample_l2).toDouble();
}

double Config::sample_l3()
{
    return settings.value(str_sample_l3).toDouble();
}

double Config::sample_S()
{
    return settings.value(str_sample_S).toDouble();
}

double Config::sample_w()
{
    return settings.value(str_sample_w).toDouble();
}

void Config::setDataDir(const QString &dirName)
{
    settings.setValue(str_data_dir, dirName);
}

void Config::setEurothermPort(const QString &port)
{
    settings.setValue(str_eurotherm_port, port);
}

void Config::setEurothermSlave(int slave)
{
    settings.setValue(str_eurotherm_slave, slave);
}

void Config::setExperimentator(const QString &name)
{
    settings.setValue(str_experimentator, name);
}

void Config::setHp34970Port(const QString &port)
{
    settings.setValue(str_hp34970_port, port);
}

void Config::setMsdpPort(const QString &port)
{
    settings.setValue(str_msdp_port, port);
}

void Config::setPs6220Port(const QString &port)
{
    settings.setValue(str_ps6220_port, port);
}

void Config::setResistivity_I(double I)
{
    settings.setValue(str_resistivity_i, I);
}

void Config::setSample_h(double h)
{
    settings.setValue(str_sample_h, h);
}

void Config::setSample_l1(double l)
{
    settings.setValue(str_sample_l1, l);
}

void Config::setSample_l2(double l)
{
    settings.setValue(str_sample_l2, l);
}

void Config::setSample_l3(double l)
{
    settings.setValue(str_sample_l3, l);
}

void Config::setSample_S(double S)
{
    settings.setValue(str_sample_S, S);
}

void Config::setSample_w(double w)
{
    settings.setValue(str_sample_w, w);
}
