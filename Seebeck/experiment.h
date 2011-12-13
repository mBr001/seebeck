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
        ERR_VALUE,
        ERR_EUROTHERM,
        ERR_MSDP,
        ERR_HP34970,
        ERR_PS6220,
        ERR_LOG_FILE,
        ERR_DIR_NOT_EXISTS
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

        /** Keithley PS 6220 precision current source port name. */
        QString ps6220Port;

        OpenParams() : eurothermSlave(-1) {}

        /** Return true if configuration values are valid. */
        bool isValid() const;
    };

    class RunParams {
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

        /** Current uset for sample heating. */
        double sampleHeatingI;

        bool isValid() const;

        RunParams() :
            furnaceHeatingOn(false), furnaceT(-274),
            furnaceSettleTStraggling(NAN), furnaceSettleTime(NAN),
            sampleHeatingI(-1)
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

        SampleParams() : l1(NAN), l2(NAN), l3(NAN), S(NAN) {}

        /** Check values for validity. */
        bool isValid() const;
    };

    class SetupParams {
    public:
        /** Current used for measurement of sample resisitvity. */
        double resistivityI;

        /** Experimantator identification, just label. */
        QString experimentator;

        /** Sample parameters. */
        SampleParams sample;

        /** Sample identifier, just label. */
        QString sampleId;

        bool isValid() const;
    };

    explicit Experiment(QObject *parent = 0);

    bool checkRunParams(const RunParams &runParams) const;
    void close();
    ExperimentError_t error() const;
    QString errorString() const;
    bool furnaceTRange(int *Tmin, int *Tmax);

    bool isRunning() const;
    bool isSetup() const;
    bool open(const OpenParams &openParams);
    bool run(const RunParams &params, int measurements = 1);
    const RunParams& runParams();

    /** Force sample T and U measurement. */
    void sampleMeasure();

    bool setup(const SetupParams &params);
    const SetupParams& setupParams() const;

    /** Abort experiment, set it from setup/running state to open.

      All devices are reset to default state. The state is changed regardless
      on return value, but when returns false, device status is undefined.
      @return: if returns true no error accured, if false error occured. */
    bool abort();

private:
    /** Column identifiers for data log CSV file. */
    typedef enum {
        COL_TIME = 0,
        COL_STATE,
        COL_FURNACE_T,
        COL_FURNACE_T_STRAGGLING,
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
        COL_SAMPLE_RES_U_FORWARD,
        COL_SAMPLE_RES_U_BACKWARD,
        // not a real column, define total number of columns
        COL_END
    } CsvLogColumns_t;

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
        HP34901_CH_RES = 114
    } HP34901Channels_t;

    typedef enum {
        HP34903_CH_I1 = 207,
        HP34903_CH_I2 = 208
    } HP34903Chennels_t;

    QCSVWriter dataLog;
    QModBus eurotherm;
    QSCPIDev hp34970;
    QSCPIDev ps6220;
    sdp_t sdp;
    sdp_va_t sdp_va_maximums;

    ExperimentError_t errorf;

    /** Directory to store files with data from experiment. */
    QDir logDir;

    bool runningf;
    RunParams runParamsf;
    int sdpError;
    SetupParams setupParamsf;
    bool setupf;

    /** Timer for measurement. */
    QTimer timer;

    /** Timing for timer. */
    static const double timerDwell;

    /** The period during which furnace temperature is stable. */
    int furnaceStableTime;
    QVector<double> furnaceTvalues;

    void doStabilize();

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
    void sampleRMeasured(double R);

private slots:
    void on_timer_timeout();

};

#endif // EXPERIMENT_H
