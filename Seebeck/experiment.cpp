#include <math.h>

#include "experiment.h"


const double Experiment::timerDwell = 1000;

bool Experiment::OpenParams::isValid() const
{
    return (eurothermSlave >= 1 && eurothermSlave <= 247);
}

bool Experiment::RunParams::isValid() const
{
    return (furnaceSettleTime >= 0 && isfinite(furnaceSettleTime) &&
            furnaceT >= -273 &&
            furnaceSettleTStraggling >= 0 && isfinite(furnaceSettleTStraggling) &&
            sampleHeatingI >= 0 && isfinite(sampleHeatingI));
}

bool Experiment::SampleParams::isValid() const
{
    return (l1 >= 0 && isfinite(l1) &&
            l2 > 0 && isfinite(l2) &&
            l3 >= 0 && isfinite(l3) &&
            S > 0 && isfinite(S));
}

bool Experiment::SetupParams::isValid() const
{
    return (sample.isValid() &&
            resistivityI > 0 && isfinite(resistivityI));
}

Experiment::Experiment(QObject *parent) :
    QObject(parent),
    dataLog(COL_END),
    errorf(ERR_OK),
    paramsf(),
    sdpError(SDP_EOK),
    state(STATE_STOP),
    timer(this)
{
    timer.setObjectName("timer");
    timer.setInterval(timerDwell);
    timer.setSingleShot(false);

    QMetaObject::connectSlotsByName(this);
}

void Experiment::abort()
{
    sdp_set_output(&sdp, 0);
    eurotherm.setEnabled(false);
}

bool Experiment::checkRunParams(const RunParams &params) const
{
    return (params.isValid() &&
            params.furnaceT <= 9999 &&
            params.sampleHeatingI <= sdp_va_maximums.curr);
}

void Experiment::close()
{
    // TODO is experiment running? -> stop
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

    sdpError = sdp_get_va_data(&sdp, &va_data);
    if (sdpError != SDP_EOK) {
        emit fatalError("doStabilize - sdp_get_va_data", sdp_strerror(sdpError));
        return;
    }
    emit sampleHeatingUIMeasured(va_data.curr, va_data.volt);
    dataLog[COL_SAMPLE_HEAT_I] = va_data.curr;
    dataLog[COL_SAMPLE_HEAT_U] = va_data.volt;

    int T;

    if (!eurotherm.currentT(&T)) {
        emit fatalError("doStabilize - eurotherm.currentT", eurotherm.errorString());
        return;
    }
    dataLog[COL_TIME] = QDateTime::currentDateTimeUtc();
    dataLog[COL_FURNACE_T] = T;
    if (!dataLog.write()) {
        // TODO
        emit fatalError("TODO", "TODO");
        return;
    }

    furnaceTvalues.push_front(T);
    furnaceTvalues.pop_back();
    double Tmean = 0.0;
    foreach (double _T_, furnaceTvalues) { Tmean += _T_; }
    Tmean /= (double)furnaceTvalues.size();
    double Ts = 0.0;
    foreach (double _T_, furnaceTvalues) {
        double dT = _T_ - Tmean;
        Ts += dT * dT;
    }
    Ts = sqrt(Ts);
    emit furnaceTMeasured(T, Ts);

    if ( fabs(T - paramsf.furnaceT) < paramsf.furnaceSettleTStraggling) {
        furnaceStableTime += timerDwell;
        if (furnaceStableTime < paramsf.furnaceSettleTime)
            return;
    }
    else {
        furnaceStableTime = 0;
        return;
    }

    //QStringList values;
    //if (!hp34970.read(&values)) {
    //} // TODO

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
    case ERR_OK:
        s = "No error";
        break;

    case ERR_INVALID_VALUE:
        s = "Invalid parameter value";
        break;

    case ERR_EUROTHERM:
        s = QString("Eurother operation error: ") + eurotherm.errorString();
        break;

    case ERR_MSDP:
        s = QString("Manson SDP power sypply error: ") + sdp_strerror(sdpError);
        break;

    case ERR_HP34970:
        s = QString("HP34970 device error:") + hp34970.errorString();
        break;

    case ERR_KEITHLEY:
        s = QString("Keithley power supply error:") /* FIXME */;
        break;

    default:
        s = "Unknown error, THIS IS WRONG.";
        break;
    }

    return s;
}

bool Experiment::furnaceTRange(int *Tmin, int *Tmax)
{
    if (!eurotherm.targetTRange(Tmin, Tmax)) {
        setError(ERR_EUROTHERM);
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

bool Experiment::open(const OpenParams &openParams)
{
    {
        const char *portName(openParams.msdpPort.toLocal8Bit().constData());
        sdpError = sdp_open(&sdp, portName, SDP_DEV_ADDR_MIN);
        if (sdpError != SDP_EOK) {
            // FIXME: do not emit error when returning value
            emit fatalError("SDP PS open failed", sdp_strerror(sdpError));
            return false;
        }
    }

    sdpError = sdp_get_va_maximums(&sdp, &sdp_va_maximums);
    if (sdpError != SDP_EOK) {
        sdp_close(&sdp);
        emit fatalError("SDP PS open failed", sdp_strerror(sdpError));
        return false;
    }

    if (!hp34970.open(openParams.hp34970Port)) {
        sdp_close(&sdp);
        emit fatalError("Open HP34970 failed", hp34970.errorString());
        return false;
    }

    QSCPIDev::Channels_t channels;

    channels.push_back(HP34901_CH_T1);
    channels.push_back(HP34901_CH_T2);
    channels.push_back(HP34901_CH_T3);
    channels.push_back(HP34901_CH_T4);
    channels.push_back(HP34901_CH_U14);
    channels.push_back(HP34901_CH_U43);
    channels.push_back(HP34901_CH_U32);
    channels.push_back(HP34901_CH_U12);
    if (!hp34970.setSense(QSCPIDev::SenseTemp, channels.mid(0, 4))) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("HP34970 T setup failed", hp34970.errorString());
        return false;
    }

    if (!hp34970.setSense(QSCPIDev::SenseVolt, channels.mid(4, 4))) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("HP34970 U setup failed", hp34970.errorString());
        return false;
    }

    if (!hp34970.setScan(channels)) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("HP34970 setup scan list", hp34970.errorString());
        return false;
    }

    if (!eurotherm.open(openParams.eurothermPort, openParams.eurothermSlave)) {
        sdp_close(&sdp);
        hp34970.close();
        emit fatalError("Failed to open Eurotherm regulator", eurotherm.errorString());
        return false;
    }

    // TODO: setup eurotherm to defined state

    QDir dataDir(openParams.dataDirName);
    if (!dataDir.exists()) {
        emit fatalError("Invalid experiment directory",
                        "Experiment data directory does not exists");
        return false;
    }
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
    dataLog[COL_TIME] = "Time\n(UTC)";
    dataLog[COL_STATE] = "State\n";
    dataLog[COL_FURNACE_T] = "Furnace T\n(°C)";
    dataLog[COL_SAMPLE_HEAT_I] = "Heat I\n(A)";
    dataLog[COL_SAMPLE_HEAT_U] = "Heat U\n(V)";
    dataLog[COL_SAMPLE_T1] = "Sample T1\n(°C)";
    dataLog[COL_SAMPLE_T2] = "Sample T\n(°C)";
    dataLog[COL_SAMPLE_T3] = "Sample T\n(°C)";
    dataLog[COL_SAMPLE_T4] = "Sample T\n(°C)";
    dataLog[COL_SAMPLE_U12] = "Sample U12\n(V)";
    dataLog[COL_SAMPLE_U23] = "Sample U23\n(V)";
    dataLog[COL_SAMPLE_U34] = "Sample U34\n(V)";
    dataLog[COL_SAMPLE_U41] = "Sample U41\n(V)";
    dataLog[COL_SAMPLE_RES_I] = "Sample res. I\n(A)";
    dataLog[COL_SAMPLE_RES_U] = "Sample res. U\n(V)";
    if (!dataLog.write()) {
        emit fatalError("CSV file write failed", "Failed to write CSV file header");
        return false;
    }

    // TODO: get from device
    //paramsf.furnacePower = ;
    //paramsf.furnaceT = ;

    sdp_va_data_t va_data;
    sdpError = sdp_get_va_data(&sdp, &va_data);
    if (sdpError != SDP_EOK) {
        sdp_close(&sdp);
        hp34970.close();
        eurotherm.close();
        emit fatalError("Failed to get I form PS", sdp_strerror(sdpError));
        return false;
    }
    paramsf.sampleHeatingI = va_data.curr;

    timer.start();

    return true;
}

Experiment::RunParams Experiment::runParams()
{
    RunParams params(paramsf);
    if (!eurotherm.targetT(&params.furnaceT)) {
        // TODO
        setError(ERR_EUROTHERM);
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

    dataLog[COL_SAMPLE_T1] = T1;
    dataLog[COL_SAMPLE_T2] = T2;
    dataLog[COL_SAMPLE_T3] = T3;
    dataLog[COL_SAMPLE_T4] = T4;

    emit sampleTMeasured(T1, T2, T3, T4);

    // FIXME: U order
    U23 = QVariant(values[4]).toDouble();
    U12 = QVariant(values[5]).toDouble();
    U34 = QVariant(values[6]).toDouble();
    U41 = QVariant(values[7]).toDouble();

    dataLog[COL_SAMPLE_U12] = U12;
    dataLog[COL_SAMPLE_U23] = U23;
    dataLog[COL_SAMPLE_U34] = U34;
    dataLog[COL_SAMPLE_U41] = U41;

    emit sampleUMeasured(U12, U23, U34, U41);
}

void Experiment::setError(ExperimentError_t error)
{
    errorf = error;
}

bool Experiment::setup(const SetupParams &params)
{
    if (!params.isValid()) {
        errorf = ERR_INVALID_VALUE;
        return false;
    }

    return false;
}

const Experiment::SetupParams& Experiment::setupParams() const
{
    return setupParamsf;
}

bool Experiment::start(const RunParams &params)
{
    if (!checkRunParams(params)) {
        setError(ERR_INVALID_VALUE);
        return false;
    }

    state = STATE_STABILIZE;
    this->paramsf = params;
    furnaceStableTime = 0;

    // Create vector for moving T avarage
    double period = 1000. / timerDwell;
    while(furnaceTvalues.size() > (period * params.furnaceSettleTime)) {
        furnaceTvalues.pop_back();
    }
    while(furnaceTvalues.size() <= (period * params.furnaceSettleTime)) {
        furnaceTvalues.push_back(0.0);
    }

    if (!eurotherm.setTarget(params.furnaceT))
    {
        // TODO
        setError(ERR_EUROTHERM);
        //emit fatalError("Failed to set up eurotherm regulator", eurotherm.errorString());
        return false;
    }

    if (!eurotherm.setEnabled(true))
    {
        // TODO
        setError(ERR_EUROTHERM);
        //emit fatalError("Failed to set up eurotherm regulator", eurotherm.errorString());
        return false;
    }

    int sdp_ret;

    sdp_ret = sdp_set_curr(&sdp, params.sampleHeatingI);
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
    abort();

    return false;
}
