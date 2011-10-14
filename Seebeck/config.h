#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

class Config
{
public:
    Config();

private:
    QSettings settings;
};

#endif // CONFIG_H
