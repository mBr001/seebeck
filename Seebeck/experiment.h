#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <math.h>
#include <QObject>
#include <QTimer>

#include "../msdptool/src/include/msdp2xxx.h"
#include "../QSCPIDev/qscpidev.h"
#include "../QCSVWriter/qcsvwriter.h"
#include "qmodbus.h"

class Experiment : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        ERR_OK = 0,
        ERR_INVALID_VALUE,
        ERR_EUROTHERM,
        ERR_MSDP,
        ERR_HP34970,
        ERR_KEITHLEY
    } ExperimentError_t;

    /** Parameter used to open experiment. */
    class OpenParams {
    public:
        /** Directory to store data taken during experiment. */
        QString dataDirName;

        /** Eurotherm regulator serial port name. */
        QString eurothermPort;

        /** Eurotherm ModBus slave number. */
        int eurothermSlave;

        /** HP34970 multimeter serial port name. */
        QString hp34970Port;

        /** Manson SDP power supply port name. */
        QString msdpPort;

        /** Return true if configuration values are valid. */
        bool isValid() const;
    };

    class RunParams_t {
    public:
        /** Turn furnace power on/off. */
        bool furnaceHeatingOn;

        /** Wanted furnace temperature (°C). */
        int furnaceT;

        /** Maximal difference from furnaceT to be system clasified as steady. */
        double furnaceSettleTStraggling;

        /** If temperature has wanted value for this period, system is
          * clasicied as stedy. */
        double furnaceSettleTime;

        /** Current used to measure sample rezistivity. */
        double rezistivityI;

        /** Current uset for sample heating. */
        double sampleHeatingI;

        RunParams_t() :
            furnaceHeatingOn(false), furnaceT(-274),
            furnaceSettleTStraggling(-1), furnaceSettleTime(-1),
            rezistivityI(NAN), sampleHeatingI(-1)
        {}
    };

    class SampleParams {
    public:
        /** Lenght from back side of sample to first measurement point. [m] */
        double l1;

        /** Distance betwen measurement points. [m] */
        double l2;

        /** Lenght from second measurement point to fron of sample. [m] */
        double l3;

        /** Surface of sample orthogonal to "lenght" axis. */
        double S;

        SampleParams();

        /** Check values for validity. */
        bool isValid() const;
    };

    explicit Experiment(QObject *parent = 0);

    void abort();
    bool checkRunParams(const RunParams_t &runParams) const;
    void close();
    ExperimentError_t error() const;
    QString errorString() const;
    bool furnaceTRange(int *Tmin, int *Tmax);

    // When changing prototype, change numeric suffix.
    // This is simple hack to keep definition clear even for function
    // which takes as parameters jus bunch of strings.
    bool open(const OpenParams &openParams);
    RunParams_t runParams();
    bool setup();

    /** Force sample T and U measurement. */
    void sampleMeasure();

    const SampleParams& sampleParams() const;
    void setSampleParams(const SampleParams& val);

    bool start(const RunParams_t &runParams);
    void stop();

private:
    /** Column identifiers for data log CSV file. */
    typedef enum {
        COL_TIME = 0,
        COL_STATE,
        COL_FURNACE_T,
        COL_SAMPLE_HEAT_I,
        COL_SAMPLE_HEAT_U,
        COL_SAMPLE_T1,
        COL_SAMPLE_T2,
        COL_SAMPLE_T3,
        COL_SAMPLE_T4,
        COL_SAMPLE_U12,
        COL_SAMPLE_U23,
        COL_SAMPLE_U34,
        COL_SAMPLE_U41,
        COL_SAMPLE_RES_I,
        COL_SAMPLE_RES_U,
        // not a real column, define number of columns
        COL_END
    } CsvLogColumns_t;

    // TODO: file to save experiment settings
    // (CSV, to catch variation during time)

    typedef enum {
        HP34901_CH_T1 = 105,
        HP34901_CH_T2 = 106,
        HP34901_CH_T3 = 107,
        HP34901_CH_T4 = 108,
        HP34901_CH_U14 = 109, // 14 ? 41
        HP34901_CH_U43 = 110, // 43 ? 34
        HP34901_CH_U32 = 111, // 32 ? 23
        HP34901_CH_U12 = 112, // 21 ? 12
        HP34901_CH_XXX = 113,
        HP34901_CH_R = 114
    } HP34901Channels_t;

    typedef enum {
        HP34903_CH_I1 = 207,
        HP34903_CH_I2 = 208
    } HP34903Chennels_t;

    /** Current state of experiment. */
    typedef enum {
        STATE_STOP = 0,
        STATE_STABILIZE,
        STATE_COOLDOWN
    } State_t;

    QCSVWriter dataLog;
    QModBus eurotherm;
    QSCPIDev hp34970;
    sdp_t sdp;
    sdp_va_t sdp_va_maximums;

    ExperimentError_t errorf;
    RunParams_t paramsf;
    SampleParams sampleParamsf;
    int sdpError;

    State_t state;
    /** Timer for measurement. */
    QTimer timer;
    /** Timing for timer. */
    static const double timerDwell;

    /** The period during which furnace temperature is stable. */
    int furnaceStableTime;
    QList<double> furnaceTvalues;

    void doCoolDown();
    void doStabilize();
    void doStop();

    void setError(ExperimentError_t error);

signals:
    void fatalError(const QString &errorShort, const QString &errorLong);
    /** Called at end of experiment, after completion all measurements. */
    void finished();

    /** Emited when T of furnace is measured. */
    void furnaceTMeasured(int T, double Tstraggling);

    /** Emited after process I and U of sample back heating is measured. */
    void sampleHeatingUIMeasured(double I, double U);

    void sampleTMeasured(double T1, double T2, double T3, double T4);
    void sampleUMeasured(double U12, double U23, double U34, double U41);

private slots:
    void on_timer_timeout();

};

#endif // EXPERIMENT_H
