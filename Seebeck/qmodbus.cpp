#include <errno.h>
#include "qmodbus.h"

QModBus::QModBus(QObject *parent) :
    QObject(parent),
    dev(NULL),
    errNo(0)
{
}

QModBus::~QModBus()
{
    if (isOpen())
        close();
}

void QModBus::close()
{
    modbus_free(dev);
    dev = NULL;
}

bool QModBus::currentT(int *T)
{
    *T = 0;
    if (modbus_read_registers(dev, REG_PV_IN, 1, (uint16_t*)T) != 1) {
        errNo = errno;
        return false;
    }
    return true;
}

int QModBus::error() const
{
    return errNo;
}

QString QModBus::errorString() const
{
    return strerror(errNo);
}

bool QModBus::isOpen() const
{
    return (dev != NULL);
}

bool QModBus::mode(bool *)
{
    return false;
}

bool QModBus::open(const QString &port, int slave)
{
    dev = modbus_new_rtu(port.toLocal8Bit().constData(), 9600, 'N', 8, 1);
    if (dev == NULL) {
        errNo = errno;
        return false;
    }

    if (modbus_connect(dev) == -1)
    {
        errNo = errno;
        return false;
    }

    if (modbus_set_slave(dev, slave) == -1) {
        errNo = errno;
        return false;
    }

    return true;
}

bool QModBus::setEnabled(bool enabled)
{
    if (modbus_write_register(dev, REG_IM, enabled ? REG_IM_AUTO : REG_IM_MANUAL) != 1) {
        errNo = errno;
        return false;
    }

    // TODO: check: set output value to 0
    if (!enabled) {
       if (modbus_write_register(dev, REG_MAN_OP, 0) != 1) {
            errNo = errno;
            return false;
        }
    }
    return true;
}

bool QModBus::setTarget(int T)
{
    if (modbus_write_register(dev, REG_SP1, T) != 1) {
        errNo = errno;
        return false;
    }

    return true;
}

bool QModBus::targetT(int *T)
{
    *T = 0;

    if (modbus_read_registers(dev, REG_TG_SP, 1, (uint16_t*)T) != 1) {
        errNo = errno;
        return false;
    }

    return true;
}

bool QModBus::targetTRange(int *Tmin, int *Tmax)
{
    *Tmin = *Tmax = 0;

    if (modbus_read_registers(dev, REG_SP_LO, 1, (uint16_t*)Tmin) != 1) {
        errNo = errno;
        return false;
    }

    if (modbus_read_registers(dev, REG_SP_HI, 1, (uint16_t*)Tmax) != 1) {
        errNo = errno;
        return false;
    }

    return true;
}
