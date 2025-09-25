/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JOB_MODEL_H
#define JOB_MODEL_H

#include <QStandardItemModel>
#include <qqmlregistration.h>

#include <cups/cups.h>
#include <kcups_export.h>

class KCupsJob;
class KCupsRequest;

class KCUPS_EXPORT JobModel : public QStandardItemModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(WhichJobs jobFilter READ jobFilter WRITE setJobFilter NOTIFY jobFilterChanged FINAL)
    Q_PROPERTY(QStringList messages READ messages NOTIFY messagesChanged FINAL)

public:
    enum Role {
        RoleJobId = Qt::UserRole + 2,
        RoleJobState,
        RoleJobStateMsg,
        RoleJobName,
        RoleJobPages,
        RoleJobSize,
        RoleJobOwner,
        RoleJobCreatedAt,
        RoleJobCompletedAt,
        RoleJobProcessedAt,
        RoleJobIconName,
        RoleJobCancelEnabled,
        RoleJobHoldEnabled,
        RoleJobReleaseEnabled,
        RoleJobRestartEnabled,
        RoleJobPrinter,
        RoleJobOriginatingHostName,
        RoleJobAuthenticationRequired
    };
    Q_ENUM(Role)

    enum JobAction {
        Cancel,
        Hold,
        Release,
        Move,
        Reprint
    };
    Q_ENUM(JobAction)

    enum WhichJobs {
        WhichAll = -1,
        WhichActive = 0,
        WhichCompleted = 1
    };
    Q_ENUM(WhichJobs)

    enum Columns {
        ColStatus = 0,
        ColName,
        ColUser,
        ColCreated,
        ColCompleted,
        ColPages,
        ColProcessed,
        ColSize,
        ColStatusMessage,
        ColPrinter,
        ColFromHost,
        LastColumn
    };
    Q_ENUM(Columns)

    explicit JobModel(QObject *parent = nullptr);

    Q_INVOKABLE void hold(const QString &printerName, int jobId);
    Q_INVOKABLE void release(const QString &printerName, int jobId);
    Q_INVOKABLE void cancel(const QString &printerName, int jobId);
    Q_INVOKABLE void restart(const QString &printerName, int jobId);
    Q_INVOKABLE void move(const QString &printerName, int jobId, const QString &toPrinterName);

    QHash<int, QByteArray> roleNames() const override;

    WhichJobs jobFilter() const;
    void setJobFilter(WhichJobs filter);

    QStringList messages() const;

private Q_SLOTS:
    void getJobs();
    void getJobFinished(KCupsRequest *request);

    void handleJobNotify(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs,
                         uint jobId,
                         uint jobState,
                         const QString &jobStateReasons,
                         const QString &jobName,
                         uint jobImpressionsCompleted);

Q_SIGNALS:
    void error(int lastError, const QString &errorTitle, const QString &errorMsg);
    void jobFilterChanged();
    void messagesChanged();
    void loaded();

private:
    int jobRow(int jobId) const;
    void insertJob(int pos, const KCupsJob &job);
    void updateJob(int pos, const KCupsJob &job);
    QString jobStatus(ipp_jstate_e job_state) const;
    void clear();
    KCupsRequest *setupRequest(std::function<void()> finished = []() { });
    void setMessages(const QString &msg);

    KCupsRequest *m_jobRequest = nullptr;
    QHash<int, QByteArray> m_roles;
    WhichJobs m_jobFilter = WhichActive;
    QStringList m_messages;
};

#endif // JOB_MODEL_H
