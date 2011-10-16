#include "experiment.h"

const double Experiment::timerDwell = 1000;

Experiment::Experiment(QObject *parent) :
    QObject(parent),
    dataLog(COL_END),
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
                         const QString &dataDirName)
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

    QDir dataDir(dataDirName);
    QString dateStr(QDateTime::currentDateTime().toString(Qt::ISODate));
    QString fileName(dateStr + ".csv");
    dataLog.setFileName(dataDir.absoluteFilePath(fileName));
    if (!dataLog.open()) {
        emit fatalError("Failed to open data log file", dataLog.errorString());
        return false;
    }
    dataLog.setAt(COL_TIME, "Time\n(UTC)");
    dataLog.setAt(COL_STATE, "State\n");
    dataLog.setAt(COL_FURNACE_T, "Furnace T\n(°C)");
    dataLog.setAt(COL_SAMPLE_T1, "Sample T1\n(°C)");
    dataLog.setAt(COL_SAMPLE_T2, "Sample T\n(°C)");
    dataLog.setAt(COL_SAMPLE_T3, "Sample T\n(°C)");
    dataLog.setAt(COL_SAMPLE_T4, "Sample T\n(°C)");
    dataLog.setAt(COL_SAMPLE_HEAT_I, "Heat I\n(A)");
    dataLog.setAt(COL_SAMPLE_HEAT_U, "Heat U\n(V)");
    dataLog.setAt(COL_SAMPLE_U12, "Sample U12\n(V)");
    dataLog.setAt(COL_SAMPLE_U23, "Sample U23\n(V)");
    dataLog.setAt(COL_SAMPLE_U34, "Sample U34\n(V)");
    dataLog.setAt(COL_SAMPLE_U41, "Sample U41\n(V)");

    // TODO: otevřít eurotherm

    return false;
}
