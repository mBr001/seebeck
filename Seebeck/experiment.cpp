#include <math.h>

#include "experiment.h"


const double Experiment::timerDwell = 1000;

Experiment::Experiment(QObject *parent) :
    QObject(parent),
    dataLog(COL_END),
    state(STATE_STOP),
    timer(this)
{
    timer.setObjectName("timer");
    timer.setInterval(timerDwell);
    timer.setSingleShot(false);

    QMetaObject::connectSlotsByName(this);
}

void Experiment::close()
{
    // TODO is expefriment running? -> stop
    timer.stop();
    sdp_close(&sdp);
    hp34970.close();
    eurotherm.close();
    dataLog.close();
}

void Experiment::doCoolDown()
{
    // set target to lovest posible/defined
    // report furnace and sample T
}

void Experiment::doStabilize()
{
    int T(eurotherm.currentT());
    dataLog.setAt(COL_TIME, QDateTime::currentDateTimeUtc());
    dataLog.setAt(COL_FURNACE_T, T);
    if (!dataLog.write()) {
       } // TODO
    emit furnaceTMeasured(T);

    if ( fabs(T - paramsf.furnaceT) < paramsf.furnaceTStraggling) {
        furnaceStableTime += timerDwell;
        if (furnaceStableTime < paramsf.furnaceSteadyTime)
            return;
    }
    else {
        furnaceStableTime = 0;
        return;
    }

    QStringList values;
    if (!hp34970.read(&values)) {
    } // TODO

    // parse values
    // write to log
    // emit signals
    // if enought measurement done emit finished
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
                         int eurothermSlave,
                         const QString &hp34970Port,
                         const QString &msdpPort,
                         const QString &dataDirName)
{
    int sdp_ret(sdp_open(&sdp, msdpPort.toLocal8Bit().constData(), SDP_DEV_ADDR_MIN));
    if (sdp_ret != SDP_EOK) {
        emit fatalError("SDP PS open failes.", sdp_strerror(sdp_ret));
        return false;
    }
    sdp_ret = sdp_get_va_maximums(&sdp, &sdp_va_maximums);
    if (sdp_ret != SDP_EOK) {
        sdp_close(&sdp);
        emit fatalError("SDP PS open failes.", sdp_strerror(sdp_ret));
        return false;
    }

    if (!hp34970.open(hp34970Port)) {
        sdp_close(&sdp);
        emit fatalError("Open HP34970 failed", hp34970.errorStr());
        return false;
    }

    // TODO: configure HP34970

    if (!eurotherm.open(eurothermPort, eurothermSlave)) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("Failed to open Eurotherm regulator.", eurotherm.errorString());
        return false;
    }

    // TODO: setup eurotherm to defined state

    QDir dataDir(dataDirName);
    QString dateStr(QDateTime::currentDateTime().toString(Qt::ISODate));
    QString fileName(dateStr + ".csv");
    dataLog.setFileName(dataDir.absoluteFilePath(fileName));
    if (!dataLog.open()) {
        sdp_close(&sdp);
        hp34970.close();
        eurotherm.close();
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

    timer.start();

    return true;
}

const Experiment::Params_t& Experiment::params() const
{
    return paramsf;
}

bool Experiment::start(const Params_t &params)
{
    if (params.furnaceSteadyTime <= 0 || !isfinite(params.furnaceSteadyTime)
            || params.furnaceT <= 0 || !isfinite(params.furnaceT)
            || params.furnaceTStraggling <= 0 || !isfinite(params.furnaceTStraggling)
            || params.sampleI <= 0 || !isfinite(params.sampleI)) {
        // TODO: report error (zavést error() a errorStrinf() ?)
        return false;
    }


    state = STATE_STABILIZE;
    this->paramsf = params;
    furnaceStableTime = 0;

    if (!eurotherm.setTarget(params.furnaceT))
    {
        //emit fatalError("Failed to set up eurotherm regulator", eurotherm.errorString());
        return false;
    }
    if (!eurotherm.setProgram(true))
    {
        //emit fatalError("Failed to set up eurotherm regulator", eurotherm.errorString());
        return false;
    }

    int sdp_ret;

    sdp_ret = sdp_set_curr(&sdp, params.sampleI);
    if (sdp_ret != SDP_EOK)
        goto sdp_err;
    sdp_ret = sdp_set_volt_limit(&sdp, sdp_va_maximums.volt);
    if (sdp_ret != SDP_EOK)
        goto sdp_err;
    sdp_ret = sdp_set_volt(&sdp, sdp_va_maximums.volt);
    if (sdp_ret != SDP_EOK)
        goto sdp_err;
    sdp_ret = sdp_set_output(&sdp, 1);
    if (sdp_ret != SDP_EOK)
        goto sdp_err;

    return true;

sdp_err:
    emit fatalError("Failed to set up SDP PS", sdp_strerror(sdp_ret));
    stop();

    return false;
}

void Experiment::stop()
{
    sdp_set_output(&sdp, 0);
    eurotherm.setProgram(false);
}
