#include "experiment.h"

const double Experiment::timerDwell = 1000;


// TODO:
// - timmer for experiment timing
// - time handling function with switch()
// - events for state changes
// - events for measurement
// - parameters seting functions

// - configuration class

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
    // TODO

    return false;
}
