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
    int currentT();
    Error_t error() const;
    QString errorString() const;
    bool open(const QString &port, int addr);
    bool setTarget(int T);
    bool setProgram(bool enabled);

private:
    modbus_t *dev;
    Error_t err;

signals:

public slots:

};

#endif // QEUROTHERM_H
