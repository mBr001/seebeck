#ifndef QEUROTHERM_H
#define QEUROTHERM_H

#include <QObject>
#include <modbus/modbus.h>

class QEurotherm : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        EOK = 0,
        ENEW,
        ECONNECT
    } Error_t;

    explicit QEurotherm(QObject *parent = 0);
    ~QEurotherm();

    void close();
    double currentT();
    Error_t error() const;
    QString errorString() const;
    bool open(const QString &port, int addr);
    bool setTarget(double T);
    bool setProgram(bool enabled);

private:
    modbus_t *dev;
    Error_t err;

signals:

public slots:

};

#endif // QEUROTHERM_H