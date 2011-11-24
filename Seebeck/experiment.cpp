#include <math.h>

#include "experiment.h"


const double Experiment::timerDwell = 1000;

Experiment::Experiment(QObject *parent) :
    QObject(parent),
    dataLog(COL_END),
    errorf(NoError),
    paramsf(),
    state(STATE_STOP),
    timer(this)
{
    timer.setObjectName("timer");
    timer.setInterval(timerDwell);
    timer.setSingleShot(false);

    QMetaObject::connectSlotsByName(this);
}

bool Experiment::checkParams(const Params_t &params) const
{
    return (params.furnaceSettleTime >= 0 && params.furnaceSettleTime < 60 * 60 * 24 * 365
            && params.furnaceT >= -273 && params.furnaceT < 5000
            && params.furnaceSettleTStraggling >= 0 && params.furnaceSettleTStraggling < 5000
            && params.sampleI >= 0 && params.sampleI <= sdp_va_maximums.curr);
}

void Experiment::close()
{
    // TODO is expefriment running? -> stop
    timer.stop();
    sdp_close(&sdp);
    hp34970.close();
    if (eurotherm.isOpen())
        eurotherm.close();
    dataLog.close();
}

void Experiment::doCoolDown()
{
    // set target to lovest posible/defined
    // report furnace and sample T
}

// TODO: co s chybama?
void Experiment::doStabilize()
{
    sdp_va_data_t va_data;

    int ret = sdp_get_va_data(&sdp, &va_data);
    if (ret != SDP_EOK) {
        emit fatalError("doStabilize - sdp_get_va_data", sdp_strerror(ret));
        return;
    }
    emit sampleHeatingUIMeasured(va_data.curr, va_data.volt);
    dataLog.setAt(COL_SAMPLE_HEAT_I, va_data.curr);
    dataLog.setAt(COL_SAMPLE_HEAT_U, va_data.volt);

    int T;

    if (!eurotherm.currentT(&T)) {
        emit fatalError("doStabilize - eurotherm.currentT", eurotherm.errorString());
        return;
    }
    dataLog.setAt(COL_TIME, QDateTime::currentDateTimeUtc());
    dataLog.setAt(COL_FURNACE_T, T);
    if (!dataLog.write()) {
       } // TODO
    emit furnaceTMeasured(T);

    if ( fabs(T - paramsf.furnaceT) < paramsf.furnaceSettleTStraggling) {
        furnaceStableTime += timerDwell;
        if (furnaceStableTime < paramsf.furnaceSettleTime)
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

Experiment::ExperimentError_t Experiment::error() const
{
    return errorf;
}

QString Experiment::errorString() const
{
    QString s;

    switch(errorf) {
    case NoError:
        s = "No error";
        break;

    case InvalidValueError:
        s = "Invalid parameter value";
        break;

    case EurothermError:
        s = "Eurother operation error";
        break;

    default:
        s = "Unknown error";
        break;
    }

    if (!errorStringf.isEmpty())
        s = s + " " + errorStringf;

    return s;
}

bool Experiment::furnaceTRange(int *Tmin, int *Tmax)
{
    if (!eurotherm.targetTRange(Tmin, Tmax)) {
        setError(EurothermError);
        return false;
    }
    return true;
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
        emit fatalError("SDP PS open failed", sdp_strerror(sdp_ret));
        return false;
    }
    sdp_ret = sdp_get_va_maximums(&sdp, &sdp_va_maximums);
    if (sdp_ret != SDP_EOK) {
        sdp_close(&sdp);
        emit fatalError("SDP PS open failed", sdp_strerror(sdp_ret));
        return false;
    }

    if (!hp34970.open(hp34970Port)) {
        sdp_close(&sdp);
        emit fatalError("Open HP34970 failed", hp34970.errorStr());
        return false;
    }

    QSCPIDev::Channels_t channels;

    channels.push_back(HP43970_CH_T1);
    channels.push_back(HP43970_CH_T2);
    channels.push_back(HP43970_CH_T3);
    channels.push_back(HP43970_CH_T4);
    channels.push_back(HP43970_CH_U14);
    channels.push_back(HP43970_CH_U43);
    channels.push_back(HP43970_CH_U32);
    channels.push_back(HP43970_CH_U12);
    if (!hp34970.setSense(QSCPIDev::SenseTemp, channels.mid(0, 4))) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("HP34970 T setup failed", eurotherm.errorString());
        return false;
    }

    if (!hp34970.setSense(QSCPIDev::SenseVolt, channels.mid(4, 4))) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("HP34970 U setup failed", eurotherm.errorString());
        return false;
    }

    if (!hp34970.setScan(channels)) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("HP34970 setup scan list", eurotherm.errorString());
        return false;
    }

    if (!eurotherm.open(eurothermPort, eurothermSlave)) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("Failed to open Eurotherm regulator", eurotherm.errorString());
        return false;
    }

    // TODO: setup eurotherm to defined state

    QDir dataDir(dataDirName);
    QString dateStr(QDateTime::currentDateTime().toString(Qt::ISODate));
    QString fileName(dateStr + "_all.csv");
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
    dataLog.setAt(COL_SAMPLE_HEAT_I, "Heat I\n(A)");
    dataLog.setAt(COL_SAMPLE_HEAT_U, "Heat U\n(V)");
    dataLog.setAt(COL_SAMPLE_T1, "Sample T1\n(°C)");
    dataLog.setAt(COL_SAMPLE_T2, "Sample T\n(°C)");
    dataLog.setAt(COL_SAMPLE_T3, "Sample T\n(°C)");
    dataLog.setAt(COL_SAMPLE_T4, "Sample T\n(°C)");
    dataLog.setAt(COL_SAMPLE_U12, "Sample U12\n(V)");
    dataLog.setAt(COL_SAMPLE_U23, "Sample U23\n(V)");
    dataLog.setAt(COL_SAMPLE_U34, "Sample U34\n(V)");
    dataLog.setAt(COL_SAMPLE_U41, "Sample U41\n(V)");

    // TODO: get from device
    //paramsf.furnacePower = ;
    //paramsf.furnaceT = ;

    sdp_va_data_t va_data;
    sdp_ret = sdp_get_va_data(&sdp, &va_data);
    if (sdp_ret != SDP_EOK) {
        sdp_close(&sdp);
        hp34970.close();
        eurotherm.close();
        emit fatalError("Failed to get I form PS", sdp_strerror(sdp_ret));
        return false;
    }
    paramsf.sampleI = va_data.curr;

    timer.start();

    return true;
}

Experiment::Params_t Experiment::params()
{
    Params_t params(paramsf);
    if (!eurotherm.targetT(&params.furnaceT)) {
        // TODO
        setError(EurothermError);
    }
    return params;
}

void Experiment::sampleMeasure()
{
    double T1, T2, T3, T4;
    double U12, U23, U34, U41;

    QStringList values;
    // FIXME: prijal nekompletni radek a pri timeoutu nepochybyl
    if (!hp34970.read(&values, 2000000)) {
        // TODO
        return;
    }
    // FIXME: check for errors
    T1 = QVariant(values[0]).toDouble();
    T2 = QVariant(values[1]).toDouble();
    T3 = QVariant(values[2]).toDouble();
    T4 = QVariant(values[3]).toDouble();

    emit sampleTMeasured(T1, T2, T3, T4);

    // FIXME: U is int but init are???
    U23 = QVariant(values[4]).toDouble();
    U12 = QVariant(values[5]).toDouble();
    U34 = QVariant(values[6]).toDouble();
    U41 = QVariant(values[7]).toDouble();

    emit sampleUMeasured(U12, U23, U34, U41);
}

void Experiment::setError(ExperimentError_t error, const QString &extraDescription)
{
    errorf = error;
    errorStringf = extraDescription;
}

bool Experiment::start(const Params_t &params)
{
    if (!checkParams(params)) {
        setError(InvalidValueError);
        return false;
    }

    state = STATE_STABILIZE;
    this->paramsf = params;
    furnaceStableTime = 0;

    if (!eurotherm.setTarget(params.furnaceT))
    {
        // TODO
        setError(EurothermError);
        //emit fatalError("Failed to set up eurotherm regulator", eurotherm.errorString());
        return false;
    }

    if (!eurotherm.setProgram(true))
    {
        // TODO
        setError(EurothermError);
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
