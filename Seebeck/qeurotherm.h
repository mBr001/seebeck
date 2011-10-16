#ifndef QEUROTHERM_H
#define QEUROTHERM_H

#include <QObject>

class QEurotherm : public QObject
{
    Q_OBJECT
public:
    explicit QEurotherm(QObject *parent = 0);

    void close();
    bool open(const QString &port, int addr);
    bool setTarget(double T);
    bool setProgram(bool enabled);

signals:

public slots:

};

#endif // QEUROTHERM_H
