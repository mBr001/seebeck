#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

class Config
{
public:
    Config();

    QString dataDir();
    QString eurothermPort();
    int eurothermSlave();
    QString hp34970Port();
    QString msdpPort();

    void setDataDir(const QString &dirName);
    void setEurothermPort(const QString &port);
    void setEurothermSlave(int slave);
    void setHp34970Port(const QString &port);
    void setMsdpPort(const QString &port);

private:
    QSettings settings;

    static const char str_data_dir[];
    static const char str_eurotherm_port[];
    static const char str_eurotherm_slave[];
    static const char str_hp34970_port[];
    static const char str_msdp_port[];
};

#endif // CONFIG_H
