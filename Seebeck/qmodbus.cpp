#include "qmodbus.h"

QModBus::QModBus(QObject *parent) :
    QObject(parent),
    dev(NULL)
{
}

QModBus::~QModBus()
{
    close();
}

void QModBus::close()
{
    if (dev != NULL) {
        modbus_free(dev);
        dev = NULL;
    }
}

int QModBus::currentT()
{
    uint16_t dest;
    modbus_read_registers(dev, 1, 1, &dest);

    return dest;
}

QModBus::Error_t QModBus::error() const
{
    return err;
}

QString QModBus::errorString() const
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

bool QModBus::open(const QString &port, int slave)
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

bool QModBus::setTarget(int)
{
    return false;
}

bool QModBus::setProgram(bool)
{
    return false;
}

bool QModBus::targetT(int *T) const
{
    *T = 0;

    return (modbus_read_registers(dev, 2, 1, (uint16_t*)T) != -1);
}
