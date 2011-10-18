#include "qmodbus.h"

QEurotherm::QEurotherm(QObject *parent) :
    QObject(parent),
    dev(NULL)
{
}

QEurotherm::~QEurotherm()
{
    close();
}

void QEurotherm::close()
{
    if (dev != NULL) {
        modbus_free(dev);
        dev = NULL;
    }
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
    case EOK:
        return "No error.";

    case ENEW:
        return "modbus_new_rtu failed";

    case ECONNECT:
        return "connect failed";

    case ESLAVE:
        return "Failed to set slave numer";

    default:
        return "FIXME";
    }
}

bool QEurotherm::open(const QString &port, int slave)
{
    err = EOK;
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

    if (modbus_set_slave(dev, slave) == -1) {
        err = ESLAVE;
        return false;
    }

    return true;
}

bool QEurotherm::setTarget(double)
{
    return false;
}

bool QEurotherm::setProgram(bool)
{
    return false;
}
