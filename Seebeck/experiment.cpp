#include "experiment.h"

const double Experiment::timerDwell = 1000;

Experiment::Experiment(QObject *parent) :
    QObject(parent),
    state(STATE_STOP),
    timer(this),
    furnaceWantedTf(NAN),
    sampleHeatIf(NAN)
{
    timer.setObjectName("timer");
    timer.setInterval(timerDwell);
    timer.setSingleShot(false);

    QMetaObject::connectSlotsByName(this);
}

void Experiment::close()
{

}

void Experiment::doCoolDown()
{
    // set target to lovest posible/defined
// report furnace and sample T
}

void Experiment::doStabilize()
{
// report
}

void Experiment::doStop()
{
// abort experiment go to cooldown?
}

void Experiment::on_timer_timeout()
{
    switch(state) {

    case STATE_COOLDOWN:
        doCoolDown(); break;

    case STATE_STABILIZE:
        doStabilize(); break;

    default:
    case STATE_STOP:
        doStop(); break;
    }
}

bool Experiment::open_00(const QString &eurothermPort,
                         const QString &hp34970Port,
                         const QString &msdpPort,
                         const QString &dataDir)
{
    if (!hp34970.open(hp34970Port)) {
        emit fatalError("Open HP34970 failed", hp34970.errorStr());
        return false;
    }

    int err(sdp_open(&sdp, msdpPort.toLocal8Bit().constData(), SDP_DEV_ADDR_MIN));
    if (err != SDP_EOK) {
        emit fatalError("SDP PS open failes.", sdp_strerror(err));
        return false;
    }

    // TODO: otevřít eurotherm
    // otevřít soubor pro logování
    //  - vložit hlavičku
    //  - pojmenovat podle data
    return false;
}
