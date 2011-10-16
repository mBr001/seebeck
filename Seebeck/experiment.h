#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <math.h>
#include <QObject>
#include <QTimer>

#include "../msdptool/src/include/msdp2xxx.h"
#include "../QSCPIDev/qscpidev.h"
#include "../QCSVWriter/qcsvwriter.h"
#include "qeurotherm.h"

class Experiment : public QObject
{
    Q_OBJECT
public:
    typedef struct {
        /** Wanted furnace temperature (°C). */
        double furnaceT;

        /** Maximal difference from furnaceT to be system clasified as steady. */
        double furnaceTStraggling;

        /** If temperature has wanted value for this period, system is
          * clasicied as stedy. */
        double furnaceSteadyTime;

        /** Current uset for sample heating. */
        double sampleI;
    } Params_t;

    explicit Experiment(QObject *parent = 0);

    void close();
    // When changing prototype, change numeric suffix.
    // This is simple hack to keep definition clear even for function
    // which takes as parameters jus bunch of strings.
    bool open_00(const QString &eurothermPort, const QString &hp34970Port,
              const QString &msdpPort, const QString &dataDirName);
    bool start(const Params_t &params);
    void stop();

private:
    /** Column identifiers for data log CSV file. */
    typedef enum {
        COL_TIME = 0,
        COL_STATE,
        COL_FURNACE_T,
        COL_SAMPLE_T1,
        COL_SAMPLE_T2,
        COL_SAMPLE_T3,
        COL_SAMPLE_T4,
        COL_SAMPLE_HEAT_I,
        COL_SAMPLE_HEAT_U,
        COL_SAMPLE_U12,
        COL_SAMPLE_U23,
        COL_SAMPLE_U34,
        COL_SAMPLE_U41,
        // not a real column, define number of columns
        COL_END
    } CsvLogColumns_t;

    // TODO: file to save experiment settings
    // (CSV, to catch variation during time)

    typedef enum {
        HP43970_CH_T1 = 5,
        HP43970_CH_T2 = 6,
        HP43970_CH_T3 = 7,
        HP43970_CH_T4 = 8,
        HP43970_CH_U14 = 9, // 14 ? 41
        HP43970_CH_U43 = 10, // 34 ? 43
        HP43970_CH_U32 = 11, // 32 ? 23
        HP43970_CH_U12 = 12 // 12 ? 21
    } Hp43970Channels_t;

    /** Current state of experiment. */
    typedef enum {
        STATE_STOP = 0,
        STATE_STABILIZE,
        STATE_COOLDOWN
    } State_t;

    QCSVFileWriter dataLog;
    QEurotherm eurotherm;
    QSCPIDev hp34970;
    sdp_t sdp;
    sdp_va_t sdp_va_maximums;

    State_t state;
    /** Timer for measurement. */
    QTimer timer;
    /** Timing for timer. */
    static const double timerDwell;

    Params_t params;

    void doCoolDown();
    void doStabilize();
    void doStop();

signals:
    void fatalError(const QString &errorShort, const QString &errorLong);
    /** Called at end of experiment, after completion all measurements. */
    void finished();

    void furnaceTMeasured(double T);
    void sampleTMeasured(double T1, double T2, double T3, double T4);
    void sampleUMeasured(double U12, double U23, double U34, double U41);

private slots:
    void on_timer_timeout();

};

#endif // EXPERIMENT_H
