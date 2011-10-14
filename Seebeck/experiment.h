#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QObject>

class Experiment : public QObject
{
    Q_OBJECT
public:
    explicit Experiment(QObject *parent = 0);

signals:

public slots:

};

#endif // EXPERIMENT_H
