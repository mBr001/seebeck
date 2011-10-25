#ifndef QEUROTHERM_H
#define QEUROTHERM_H

#include <QObject>
#include <modbus/modbus.h>

class QModBus : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        EOK = 0,
        ENEW,
        ECONNECT,
        ESLAVE
    } Error_t;

    explicit QModBus(QObject *parent = 0);
    ~QModBus();

    void close();
    bool currentT(int *T);
    Error_t error() const;
    QString errorString() const;
    bool isOpen() const;
    bool open(const QString &port, int addr);
    bool setTarget(int T);
    bool setProgram(bool enabled);
    bool targetT(int *T) const;
    bool targetTRange(int *Tmin, int *Tmax);

private:
    typedef enum {
        /** Temperature input value. */
        REG_PV_IN = 1,
        /** Target setpoint. */
        REG_TG_SP = 2,
        /** Manual output value. */
        REG_MAN_OP = 3,
        /** Active setpoint select: { 0 = SP1, 1 = SP2 }. */
        REG_SP_SEL = 15,
        /** Setpoint 1. */
        REG_SP1 = 24,
        /** Setpoint 2. */
        REG_SP2 = 25,
        /** Setpoint high limit. */
        REG_SP_HI = 111,
        /** Setpoint low limit. */
        REG_SP_LO = 112,
        /** Instrument mode: 0 - auto, 1 - manual, 2 - standby */
        REG_IM = 199,
        /** Loop break. */
        REG_LBR = 263,
        /** Mode of loop: { 0 = Auto, 1 = manual }. */
        REG_A_M = 273
    } Registers;

    modbus_t *dev;
    Error_t err;

signals:

public slots:

};

#endif // QEUROTHERM_H
