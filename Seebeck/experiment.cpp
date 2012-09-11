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
    errorf(ERR_OK),
    runningf(false),
    sdpError(SDP_EOK),
    setupf(false),
    timer(this)
{
    timer.setObjectName("timer");
    timer.setInterval(timerDwell);
    timer.setSingleShot(false);

    QMetaObject::connectSlotsByName(this);
}

bool Experiment::abort()
{
    bool ok(true);

    setupf = runningf = false;
    sdpError = sdp_set_output(&sdp, false);
    if (sdpError != SDP_EOK) {
        errorf = ERR_MSDP;
        ok = false;
    }

    if (!eurotherm.setEnabled(false)) {
        errorf = ERR_EUROTHERM;
        ok = false;
    }

    return ok;
}

bool Experiment::checkRunParams(const RunParams &params) const
{
    return (params.isValid() &&
            params.furnaceT <= 9999 &&
            params.sampleHeatingI <= sdp_va_maximums.curr);
}

void Experiment::close()
{
    if (setupf || runningf)
        abort();
    timer.stop();
    sdp_close(&sdp);
    ps6220.close();
    hp34970.close();
    eurotherm.close();
    dataLog.close();
}

void Experiment::doStabilize()
{
    sdp_va_data_t va_data;

    sdpError = sdp_get_va_data(&sdp, &va_data);
    if (sdpError != SDP_EOK) {
        errorf = ERR_MSDP;
        emit fatalError("doStabilize - sdp_get_va_data", sdp_strerror(sdpError));
        return;
    }
    emit sampleHeatingUIMeasured(va_data.curr, va_data.volt);

    if (!isSetup())
        return;

    int furnaceT;

    if (!eurotherm.currentT(&furnaceT)) {
        errorf = ERR_EUROTHERM;
        emit fatalError("doStabilize - eurotherm.currentT", eurotherm.errorString());
        return;
    }

    furnaceTvalues.push_front(furnaceT);
    furnaceTvalues.pop_back();
    double Tstraggling(0.);
    foreach (double Tn, furnaceTvalues) {
        double dT(Tn - runParamsf.furnaceT);
        Tstraggling += dT * dT;
    }
    Tstraggling = sqrt(Tstraggling);
    emit furnaceTMeasured(furnaceT, Tstraggling);

    if ( Tstraggling <= runParamsf.furnaceSettleTStraggling) {
        furnaceStableTime += timerDwell;
        if (furnaceStableTime < runParamsf.furnaceSettleTime)
            return;
    }
    else {
        furnaceStableTime = 0;
        return;
    }

    if (!isRunning())
        return;

    // measurement aparature temperature is stable, do measurement(s)
    dataLog[COL_SAMPLE_HEAT_I] = va_data.curr;
    dataLog[COL_SAMPLE_HEAT_U] = va_data.volt;
    dataLog[COL_TIME] = QDateTime::currentDateTimeUtc();
    dataLog[COL_FURNACE_T] = furnaceT;
    dataLog[COL_FURNACE_T_STRAGGLING] = Tstraggling;

    sampleMeasure();

    if (!dataLog.write()) {
        errorf = ERR_LOG_FILE;
        emit fatalError("doStabilize - failed to write data", dataLog.errorString());
        return;
    }

    runningf = false;

    emit runCompleted();
}

Experiment::ExperimentError_t Experiment::error() const
{
    return errorf;
}

QString Experiment::errorString() const
{
    switch(errorf) {
    case ERR_OK:
        return "No error";

    case ERR_VALUE:
        return "Invalid parameter value";
        break;

    case ERR_EUROTHERM:
        return QString("Eurother operation error: ") + eurotherm.errorString();
        break;

    case ERR_MSDP:
        return QString("Manson SDP power sypply error: ") + sdp_strerror(sdpError);
        break;

    case ERR_HP34970:
        return QString("HP34970 device error: ") + hp34970.errorString();
        break;

    case ERR_PS6220:
        return QString("Keithley power supply error: ") + ps6220.errorString();
        break;

    case ERR_LOG_FILE:
        return QString("Data logging failed: ") + dataLog.errorString();
        break;

    case ERR_DIR_NOT_EXISTS:
        return QString("Data log directory does not exists");

    default:
        return "Unknown error, THIS IS WRONG.";
        break;
    }
}

bool Experiment::furnaceTRange(int *Tmin, int *Tmax)
{
    if (!eurotherm.targetTRange(Tmin, Tmax)) {
        errorf = ERR_EUROTHERM;
        return false;
    }
    return true;
}

bool Experiment::isRunning() const
{
    return runningf;
}

bool Experiment::isSetup() const
{
    return setupf;
}

void Experiment::on_timer_timeout()
{
    doStabilize();
}

bool Experiment::open(const OpenParams &openParams)
{
    {
        const char *portName(openParams.msdpPort.toLocal8Bit().constData());
        sdpError = sdp_open(&sdp, portName, SDP_DEV_ADDR_MIN);
        if (sdpError != SDP_EOK) {
            errorf = ERR_MSDP;
            return false;
        }
    }

    sdpError = sdp_get_va_maximums(&sdp, &sdp_va_maximums);
    if (sdpError != SDP_EOK) {
        errorf = ERR_MSDP;
        sdp_close(&sdp);
        return false;
    }

    if (!hp34970.open(openParams.hp34970Port)) {
        errorf = ERR_HP34970;
        sdp_close(&sdp);
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
    channels.push_back(HP34901_CH_RES);
    if (!hp34970.setSense(QSCPIDev::SenseTemp, channels.mid(0, 4))) {
        errorf = ERR_HP34970;
        sdp_close(&sdp);
        hp34970.close();
        return false;
    }

    if (!hp34970.setSense(QSCPIDev::SenseVolt, channels.mid(4, 5))) {
        errorf = ERR_HP34970;
        sdp_close(&sdp);
        hp34970.close();
        return false;
    }

    if (!hp34970.setRoute(QSCPIDev::Channels_t(), HP34901_CH_MIN, HP34901_CH_MAX)) {
        errorf = ERR_HP34970;
        sdp_close(&sdp);
        hp34970.close();
        return false;
    }

    if (!ps6220.open(openParams.ps6220Port, QSerial::Baude19200)) {
        errorf = ERR_PS6220;
        sdp_close(&sdp);
        hp34970.close();
        return false;
    }

    if (!ps6220.setOutput(false)) {
        errorf = ERR_PS6220;
        return false;
    }

    if (!eurotherm.open(openParams.eurothermPort, openParams.eurothermSlave)) {
        errorf = ERR_EUROTHERM;
        sdp_close(&sdp);
        hp34970.close();
        ps6220.close();
        emit fatalError("Failed to open Eurotherm regulator", eurotherm.errorString());
        return false;
    }

    // TODO: setup eurotherm to defined state

    logDir.setPath(openParams.dataDirName);
    if (!logDir.exists()) {
        errorf = ERR_DIR_NOT_EXISTS;
        sdp_close(&sdp);
        hp34970.close();
        ps6220.close();
        eurotherm.close();

        return false;
    }

    sdp_va_data_t va_data;
    sdpError = sdp_get_va_data(&sdp, &va_data);
    if (sdpError != SDP_EOK) {
        errorf = ERR_MSDP;
        sdp_close(&sdp);
        hp34970.close();
        ps6220.close();
        eurotherm.close();
        emit fatalError("Failed to get I form PS", sdp_strerror(sdpError));
        return false;
    }
    runParamsf.sampleHeatingI = va_data.curr;

    timer.start();

    return true;
}

const Experiment::RunParams& Experiment::runParams()
{
    return runParamsf;
}

void Experiment::sampleMeasure()
{
    double T1, T2, T3, T4;
    double U12, U23, U34, U41;

    QSCPIDev::Channels_t channels;
    channels.push_back(HP34901_CH_T1);
    channels.push_back(HP34901_CH_T2);
    channels.push_back(HP34901_CH_T3);
    channels.push_back(HP34901_CH_T4);
    channels.push_back(HP34901_CH_U14);
    channels.push_back(HP34901_CH_U43);
    channels.push_back(HP34901_CH_U32);
    channels.push_back(HP34901_CH_U12);

    if (!hp34970.setScan(channels)) {
        errorf = ERR_HP34970;
        emit fatalError("todo", "todo");
        return;
    }

    QStringList values;
    // FIXME: prijal nekompletni radek a pri timeoutu nenahlasil chybu
    if (!hp34970.read(&values, 2000000)) {
        errorf = ERR_HP34970;
        emit fatalError("todo", "todo");
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

    if (!ps6220.setCurrent(setupParamsf.resistivityI)) {
        errorf = ERR_PS6220;
        emit fatalError("todo", "todo");
        return;
    }

    if (!ps6220.setOutput(true)) {
        errorf = ERR_PS6220;
        emit fatalError("todo", "todo");
        return;
    }

    channels.clear();
    channels.push_back(HP34903_CH_I1);
    channels.push_back(HP34903_CH_I2);
    if (!hp34970.setRoute(channels, HP34903_CH_MIN, HP34903_CH_MAX)) {
        errorf = ERR_HP34970;
        emit fatalError("todo", "todo");
        return;
    }

    if (!hp34970.setScan(HP34901_CH_RES)) {
        errorf = ERR_HP34970;
        emit fatalError("todo", "todo");
        return;
    }

    if (!hp34970.read(&values)) {
        errorf = ERR_HP34970;
        emit fatalError("todo", "todo");
        return;
    }
    double resU1(QVariant(values[0]).toDouble());
    dataLog[COL_SAMPLE_RES_U_FORWARD] = resU1;

    if (!ps6220.setCurrent(-setupParamsf.resistivityI)) {
        errorf = ERR_PS6220;
        emit fatalError("todo", "todo");
        return;
    }

    if (!hp34970.read(&values)) {
        errorf = ERR_HP34970;
        emit fatalError("todo", "todo");
        return;
    }
    double resU2(QVariant(values[0]).toDouble());
    dataLog[COL_SAMPLE_RES_U_BACKWARD] = resU2;

    if (!ps6220.setOutput(false)) {
        errorf = ERR_PS6220;
        emit fatalError("todo", "todo");
        return;
    }

    double l(setupParamsf.sample.l1 + setupParamsf.sample.l2 + setupParamsf.sample.l3);
    emit sampleRMeasured(fabs(resU1-resU2) / setupParamsf.resistivityI / setupParamsf.sample.S * l);
}

bool Experiment::setup(const SetupParams &params)
{
    if (!params.isValid()) {
        errorf = ERR_VALUE;
        return false;
    }

    // check if values are not out of sensible borders
    if (params.resistivityI > 0.05 ||
            params.sample.l1 > 0.3 ||
            params.sample.l2 > 0.3 ||
            params.sample.l3 > 0.3 ||
            params.sample.S > (0.1*0.1)) {
        errorf = ERR_VALUE;
        return false;
    }

    setupParamsf = params;

    errorf = ERR_LOG_FILE;
    QString dateStr(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    QString fileName(dateStr + "_seebeck.csv");
    dataLog.setFileName(logDir.absoluteFilePath(fileName));
    if (!dataLog.open())
        return false;

    dataLog.resize(1);
    dataLog[0] = "Seebeck experiment measurement data log";
    if (!dataLog.write())
        return false;

    dataLog.resize(2);
    dataLog[0] = "Measured by";
    dataLog[1] = params.experimentator;
    if (!dataLog.write())
        return false;

    dataLog[0] = "Sample ID";
    dataLog[1] = params.sampleId;
    if (!dataLog.write())
        return false;

    dataLog[0] = "Date (UTC)";
    dataLog[1] = dateStr;
    if (!dataLog.write())
        return false;

    dataLog[0] = "sample l1 [m]";
    dataLog[1] = params.sample.l1;
    if (!dataLog.write())
        return false;

    dataLog[0] = "sample l2 [m]";
    dataLog[1] = params.sample.l2;
    if (!dataLog.write())
        return false;

    dataLog[0] = "sample l3 [m]";
    dataLog[1] = params.sample.l3;
    if (!dataLog.write())
        return false;

    dataLog[0] = "sample S [m^2]";
    dataLog[1] = params.sample.S;
    if (!dataLog.write())
        return false;

    dataLog[0] = "I for resistivity meas. [A]";
    dataLog[1] = params.resistivityI;
    if (!dataLog.write())
        return false;

    // empty row separate document header from data
    dataLog.resize(0);
    if (!dataLog.write())
        return false;

    dataLog.resize(COL_END);
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
    dataLog[COL_SAMPLE_RES_U_FORWARD] = "Sample res. U forw.\n(V)";
    dataLog[COL_SAMPLE_RES_U_BACKWARD] = "Sample res. U back.\n(V)";
    if (!dataLog.write())
        return false;

    errorf = ERR_OK;
    setupf = true;

    return true;
}

const Experiment::SetupParams& Experiment::setupParams() const
{
    return setupParamsf;
}

// TODO support multiple measurements (current only 0 and 1)
bool Experiment::run(const RunParams &params, int measurements)
{
    if (!checkRunParams(params)) {
        errorf = ERR_VALUE;
        return false;
    }

    if (params.furnaceSettleTime != runParamsf.furnaceSettleTime)
    {
        furnaceStableTime = 0;

        // Create vector for moving T avarage
        double period = 1000. / timerDwell;
        int n(period * params.furnaceSettleTime);
        if (furnaceTvalues.size() < n)
            furnaceTvalues.insert(furnaceTvalues.size(), n - furnaceTvalues.size(), -274.);
        else
            furnaceTvalues.resize(n);
    }

    if (params.furnaceT != runParamsf.furnaceT ||
            params.furnaceHeatingOn != runParamsf.furnaceHeatingOn) {
        errorf = ERR_EUROTHERM;
        furnaceStableTime = 0;

        if (params.furnaceHeatingOn) {
            if (!eurotherm.setTarget(params.furnaceT))
                return false;
        }

        if (!eurotherm.setEnabled(params.furnaceHeatingOn))
            return false;
    }

    if (params.sampleHeatingI != runParamsf.sampleHeatingI) {
        errorf = ERR_MSDP;
        furnaceStableTime = 0;

        sdpError = sdp_set_curr(&sdp, params.sampleHeatingI);
        if (sdpError != SDP_EOK)
            return false;
        // todo move this into setup
        sdpError = sdp_set_volt_limit(&sdp, sdp_va_maximums.volt);
        if (sdpError != SDP_EOK)
            return false;
        sdpError = sdp_set_volt(&sdp, sdp_va_maximums.volt);
        if (sdpError != SDP_EOK)
            return false;
        sdpError = sdp_set_output(&sdp, 1);
        if (sdpError != SDP_EOK)
            return false;
    }

    this->runParamsf = params;
    if (measurements > 0)
        runningf = true;

    errorf = ERR_OK;
    return true;
}
