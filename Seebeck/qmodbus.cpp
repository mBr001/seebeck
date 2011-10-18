#include "qmodbus.h"

QEurotherm::QEurotherm(QObject *parent) :
    QObject(parent)
{
}

QEurotherm::~QEurotherm()
{
    modbus_free(dev);
}

void QEurotherm::close()
{

}

double QEurotherm::currentT()
{
    return -300;
}

QEurotherm::Error_t QEurotherm::error() const
{
    return err;
}

QString QEurotherm::errorString() const
{
    switch(err) {
    case ENEW:
        return "modbus_new_rtu failed";

    case ECONNECT:
        return "connect failed";

    default:
        return "FIXME";
    }
}

bool QEurotherm::open(const QString &port, int)
{
    dev = modbus_new_rtu(port.toLocal8Bit().constData(), 9600, 'N', 8, 1);
    if (dev == NULL) {
        err = ENEW;
        return false;
    }
    if (modbus_connect(dev) == -1)
    {
        err = ECONNECT;
        return false;
    }
    return false;
}

bool QEurotherm::setTarget(double)
{
    return false;
}

bool QEurotherm::setProgram(bool)
{
    return false;
}
