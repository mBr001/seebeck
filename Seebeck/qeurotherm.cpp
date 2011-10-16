#include "qeurotherm.h"

QEurotherm::QEurotherm(QObject *parent) :
    QObject(parent)
{
}

void QEurotherm::close()
{

}

double QEurotherm::currentT()
{
    return -300;
}

QString QEurotherm::errorString()
{
    return "FIXME";
}

bool QEurotherm::open(const QString &, int)
{
    return false;
}

bool QEurotherm::setTarget(double)
{
    return false;
}

bool QEurotherm::setProgram(bool enabled)
{
    return false;
}
