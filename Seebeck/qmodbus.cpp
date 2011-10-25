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

bool QModBus::currentT(int *T)
{
    *T = 0;
    return (modbus_read_registers(dev, REG_PV_IN, 1, (uint16_t*)T) == 1);
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

bool QModBus::isOpen() const
{
    return (dev != NULL);
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

bool QModBus::setTarget(int T)
{
    //return (modbus_write_register(dev, REG_SP_SEL, 0) == 1);
    return (modbus_write_registers(dev, REG_SP1, 1, (uint16_t*)&T) == 1);
}

bool QModBus::setProgram(bool enabled)
{
    return (modbus_write_register(dev, REG_IM, enabled ? 1 : 0) == 1);
}

bool QModBus::targetT(int *T) const
{
    *T = 0;

    return (modbus_read_registers(dev, REG_TG_SP, 1, (uint16_t*)T) == 1);
}

bool QModBus::targetTRange(int *Tmin, int *Tmax)
{
    *Tmin = *Tmax = 0;

    if (modbus_read_registers(dev, REG_SP_LO, 1, (uint16_t*)Tmin) != 1)
        return false;
    if (modbus_read_registers(dev, REG_SP_HI, 1, (uint16_t*)Tmax) != 1)
        return false;
    return true;
}
