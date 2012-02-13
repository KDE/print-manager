#ifndef KCUPSJOB_H
#define KCUPSJOB_H

#include <QString>
#include <KCupsConnection.h>

class KCupsJob
{
public:
    KCupsJob(int jobId, const QString &printer);

    int jobId() const;
    QString printer() const;
    QString location() const;
    QString info() const;
    QString makeAndModel() const;
    QStringList commands() const;

    int state() const;
    QString stateMsg() const;
    int markerChangeTime() const;

protected:
    KCupsJob(const Arguments &arguments);

private:
    friend class KCupsRequestServer;

    int     m_jobId;
    QString m_printer;
    Arguments m_arguments;
};

#endif // KCUPSJOB_H
