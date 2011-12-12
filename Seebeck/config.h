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
    QString experimentator();
    QString hp34970Port();
    QString ps6220Port();
    QString msdpPort();
    double resistivity_I();
    double sample_h();
    double sample_l1();
    double sample_l2();
    double sample_l3();
    double sample_S();
    double sample_w();

    void setDataDir(const QString &dirName);
    void setEurothermPort(const QString &port);
    void setEurothermSlave(int slave);
    void setExperimentator(const QString &name);
    void setHp34970Port(const QString &port);
    void setPs6220Port(const QString &port);
    void setMsdpPort(const QString &port);
    void setResistivity_I(double I);
    void setSample_h(double h);
    void setSample_l1(double l);
    void setSample_l2(double l);
    void setSample_l3(double l);
    void setSample_S(double S);
    void setSample_w(double w);

private:
    QSettings settings;

    static const char str_data_dir[];
    static const char str_eurotherm_port[];
    static const char str_eurotherm_slave[];
    static const char str_hp34970_port[];
    static const char str_msdp_port[];
    static const char str_ps6220_port[];
    static const char str_sample_l1[];
    static const char str_sample_l2[];
    static const char str_sample_l3[];
    static const char str_sample_S[];
    static const char str_sample_h[];
    static const char str_resistivity_i[];
    static const char str_sample_w[];
    static const char str_experimentator[];
};

#endif // CONFIG_H
