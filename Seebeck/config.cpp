#include "config.h"

const char Config::str_data_dir[] = "data dir";
const char Config::str_eurotherm_port[] = "eurother port";
const char Config::str_eurotherm_slave[] = "eurother slave";
const char Config::str_hp34970_port[] = "hp34970 port";
const char Config::str_msdp_port[] = "msdp port";
const char Config::str_ps6220_port[] = "ps 6220 port";

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
