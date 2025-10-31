/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "JobModel.h"

#include "kcupslib_log.h"

#include <KCupsJob.h>
#include <KCupsPrinter.h>
#include <KCupsRequest.h>

#include <QDateTime>

#include <KFormat>
#include <KLocalizedString>
#include <KUser>

using namespace Qt::StringLiterals;

JobModel::JobModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setHorizontalHeaderItem(ColStatus, new QStandardItem(i18n("Status")));
    setHorizontalHeaderItem(ColName, new QStandardItem(i18n("Name")));
    setHorizontalHeaderItem(ColUser, new QStandardItem(i18n("User")));
    setHorizontalHeaderItem(ColCreated, new QStandardItem(i18n("Created")));
    setHorizontalHeaderItem(ColCompleted, new QStandardItem(i18n("Completed")));
    setHorizontalHeaderItem(ColPages, new QStandardItem(i18n("Pages")));
    setHorizontalHeaderItem(ColProcessed, new QStandardItem(i18n("Processed")));
    setHorizontalHeaderItem(ColSize, new QStandardItem(i18n("Size")));
    setHorizontalHeaderItem(ColStatusMessage, new QStandardItem(i18n("Status Message")));
    setHorizontalHeaderItem(ColPrinter, new QStandardItem(i18n("Printer")));
    setHorizontalHeaderItem(ColFromHost, new QStandardItem(i18n("From Hostname")));

    m_roles = QStandardItemModel::roleNames();
    m_roles[RoleJobId] = "jobId";
    m_roles[RoleJobState] = "jobState";
    m_roles[RoleJobStateMsg] = "jobStateMsg";
    m_roles[RoleJobName] = "jobName";
    m_roles[RoleJobPages] = "jobPages";
    m_roles[RoleJobSize] = "jobSize";
    m_roles[RoleJobOwner] = "jobOwner";
    m_roles[RoleJobCreatedAt] = "jobCreatedAt";
    m_roles[RoleJobCompletedAt] = "jobCompletedAt";
    m_roles[RoleJobProcessedAt] = "jobProcessedAt";
    m_roles[RoleJobIconName] = "jobIconName";
    m_roles[RoleJobCancelEnabled] = "jobCancelEnabled";
    m_roles[RoleJobHoldEnabled] = "jobHoldEnabled";
    m_roles[RoleJobReleaseEnabled] = "jobReleaseEnabled";
    m_roles[RoleJobRestartEnabled] = "jobRestartEnabled";
    m_roles[RoleJobPrinter] = "jobPrinter";
    m_roles[RoleJobOriginatingHostName] = "jobFrom";
    m_roles[RoleJobAuthenticationRequired] = "jobAuthRequired";

    // This is emitted when a job change it's state
    connect(KCupsConnection::global(), &KCupsConnection::jobState, this, &JobModel::handleJobNotify);

    // This is emitted when a job is created
    connect(KCupsConnection::global(), &KCupsConnection::jobCreated, this, &JobModel::handleJobNotify);

    // This is emitted when a job is stopped
    connect(KCupsConnection::global(), &KCupsConnection::jobStopped, this, &JobModel::handleJobNotify);

    // This is emitted when a job has it's config changed
    connect(KCupsConnection::global(), &KCupsConnection::jobConfigChanged, this, &JobModel::handleJobNotify);

    // This is emitted when a job change it's progress
    connect(KCupsConnection::global(), &KCupsConnection::jobProgress, this, &JobModel::handleJobNotify);

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(), &KCupsConnection::jobCompleted, this, &JobModel::handleJobNotify);

    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &JobModel::getJobs);

    getJobs();
}

void JobModel::hold(const QString &printerName, int jobId)
{
    const auto request = setupRequest();
    request->holdJob(printerName, jobId);
}

void JobModel::release(const QString &printerName, int jobId)
{
    const auto request = setupRequest();
    request->releaseJob(printerName, jobId);
}

void JobModel::restart(const QString &printerName, int jobId)
{
    const auto request = setupRequest();
    request->restartJob(printerName, jobId);
}

void JobModel::cancel(const QString &printerName, int jobId)
{
    const auto request = setupRequest();
    request->cancelJob(printerName, jobId);
}

void JobModel::move(const QString &printerName, int jobId, const QString &toPrinterName)
{
    const auto request = setupRequest();
    request->moveJob(printerName, jobId, toPrinterName);
}

void JobModel::getJobs()
{
    const static QStringList attrs({KCUPS_JOB_ID,
                                    KCUPS_JOB_NAME,
                                    KCUPS_JOB_K_OCTETS,
                                    KCUPS_JOB_K_OCTETS_PROCESSED,
                                    KCUPS_JOB_STATE,
                                    KCUPS_JOB_STATE_REASONS,
                                    KCUPS_JOB_HOLD_UNTIL,
                                    KCUPS_TIME_AT_COMPLETED,
                                    KCUPS_TIME_AT_CREATION,
                                    KCUPS_TIME_AT_PROCESSING,
                                    KCUPS_JOB_PRINTER_URI,
                                    KCUPS_JOB_ORIGINATING_USER_NAME,
                                    KCUPS_JOB_ORIGINATING_HOST_NAME,
                                    KCUPS_JOB_MEDIA_PROGRESS,
                                    KCUPS_JOB_MEDIA_SHEETS,
                                    KCUPS_JOB_MEDIA_SHEETS_COMPLETED,
                                    KCUPS_JOB_PRINTER_STATE_MESSAGE,
                                    KCUPS_JOB_PRESERVED});

    if (m_jobRequest) {
        return;
    }

    m_messages.clear();
    m_jobRequest = new KCupsRequest;
    connect(m_jobRequest, &KCupsRequest::finished, this, &JobModel::getJobFinished);
    m_jobRequest->getJobs(QString(), false, m_jobFilter, attrs);
}

static KCupsJobs sanitizeJobs(KCupsJobs jobs)
{
    // For some reason sometimes cups has broken job queues with jobs with duplicated id
    // our model doesn't like that at all so sanitize the job list before processing it
    QVector<int> seenIds;
    int i = 0;
    while (i < jobs.count()) {
        const int jobId = jobs.at(i).id();
        if (seenIds.contains(jobId)) {
            qCWarning(LIBKCUPS) << "Found job with duplicated id" << jobId;
            jobs.removeAt(i);
        } else {
            seenIds << jobId;
            ++i;
        }
    }
    return jobs;
}

void JobModel::getJobFinished(KCupsRequest *request)
{
    if (request) {
        if (request->hasError()) {
            clear();
        } else {
            QStringList msgList;
            const auto jobs = sanitizeJobs(request->jobs());
            for (int i = 0; i < jobs.size(); ++i) {
                const auto job = jobs.at(i);
                if (job.state() == IPP_JOB_PROCESSING) {
                    msgList << u"Processing Job\t%1 [%2]"_s.arg(job.name(), job.printer());
                }

                // try to find the job row
                const int job_row = jobRow(job.id());
                if (job_row == -1) {
                    // not found, insert new one
                    insertJob(i, job);
                } else {
                    // update the job
                    updateJob(job_row, job);

                    if (job_row != i) {
                        // found at wrong position
                        // take it and insert on the right position
                        const QList<QStandardItem *> row = takeRow(job_row);
                        insertRow(i, row);
                    }
                }
            }

            // remove old jobs
            // The above code starts from 0 and make sure
            // dest == modelIndex(x) and if it's not the
            // case it either inserts or moves it.
            // so any item > num_jobs can be safely deleted
            while (rowCount() > jobs.size()) {
                removeRow(rowCount() - 1);
            }

            if (!msgList.empty()) {
                setMessages(msgList);
            }
        }
        request->deleteLater();
    } else {
        qCWarning(LIBKCUPS) << "Should not be called from a non KCupsRequest class" << sender();
    }
    m_jobRequest = nullptr;
    Q_EMIT loaded();
}

void JobModel::handleJobNotify(const QString &text,
                               const QString &printerUri,
                               const QString &printerName,
                               uint printerState,
                               const QString &printerStateReasons,
                               bool printerIsAcceptingJobs,
                               uint jobId,
                               uint jobState,
                               const QString &jobStateReasons,
                               const QString &jobName,
                               uint jobImpressionsCompleted)
{
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerName)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)
    Q_UNUSED(jobId)
    Q_UNUSED(jobState)
    Q_UNUSED(jobStateReasons)
    Q_UNUSED(jobName)
    Q_UNUSED(jobImpressionsCompleted)

    qCWarning(LIBKCUPS) << "JOBNOTIFY" << printerName << jobId << jobState << jobStateReasons;
    getJobs();
}

void JobModel::insertJob(int pos, const KCupsJob &job)
{
    // insert the first column which has the job state and id
    QList<QStandardItem *> row;
    ipp_jstate_e jobState = job.state();
    auto statusItem = new QStandardItem(jobStatus(jobState));
    statusItem->setData(jobState, RoleJobState);
    statusItem->setData(job.stateMsg(), RoleJobStateMsg);
    statusItem->setData(job.id(), RoleJobId);
    statusItem->setData(job.name(), RoleJobName);
    statusItem->setData(job.originatingUserName(), RoleJobOwner);
    statusItem->setData(job.originatingHostName(), RoleJobOriginatingHostName);
    QString size = KFormat().formatByteSize(job.size());
    statusItem->setData(size, RoleJobSize);

    const auto createdAt = QLocale().toString(job.createdAt(), QStringLiteral("ddd MMMM d yyyy"));
    statusItem->setData(createdAt, RoleJobCreatedAt);
    const auto completedAt = QLocale().toString(job.completedAt(), QStringLiteral("ddd MMMM d yyyy"));
    statusItem->setData(completedAt, RoleJobCompletedAt);
    const auto processedAt = QLocale().toString(job.processedAt(), QStringLiteral("ddd MMMM d yyyy"));
    statusItem->setData(processedAt, RoleJobProcessedAt);

    // TODO move the update code before the insert and reuse some code...
    statusItem->setData(KCupsJob::iconName(jobState), RoleJobIconName);
    statusItem->setData(KCupsJob::cancelEnabled(jobState), RoleJobCancelEnabled);
    statusItem->setData(KCupsJob::holdEnabled(jobState), RoleJobHoldEnabled);
    statusItem->setData(KCupsJob::releaseEnabled(jobState), RoleJobReleaseEnabled);
    statusItem->setData(job.reprintEnabled(), RoleJobRestartEnabled);

    QString pages = QString::number(job.pages());
    if (job.processedPages()) {
        pages = QString::number(job.processedPages()) + QLatin1Char('/') + QString::number(job.pages());
    }
    if (statusItem->data(RoleJobPages) != pages) {
        statusItem->setData(pages, RoleJobPages);
    }

    row << statusItem;
    for (int i = ColName; i < LastColumn; i++) {
        // adds all Items to the model
        row << new QStandardItem;
    }

    // insert the whole row
    insertRow(pos, row);

    // update the items
    updateJob(pos, job);
}

KCupsRequest *JobModel::setupRequest(std::function<void()> finished)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, finished](KCupsRequest *r) {
        if (r->hasError()) {
            Q_EMIT error(r->error(), r->serverError(), r->errorMsg());
        } else {
            finished();
        }
        r->deleteLater();
    });

    return request;
}

void JobModel::setMessages(const QStringList &list)
{
    m_messages << list;
    Q_EMIT messagesChanged();
}

void JobModel::updateJob(int pos, const KCupsJob &job)
{
    // Job Status & internal dataipp_jstate_e
    ipp_jstate_e jobState = job.state();
    QStandardItem *colStatus = item(pos, ColStatus);
    if (colStatus->data(RoleJobState).toInt() != jobState) {
        colStatus->setText(jobStatus(jobState));
        colStatus->setData(static_cast<int>(jobState), RoleJobState);
        colStatus->setData(job.stateMsg(), RoleJobStateMsg);

        colStatus->setData(KCupsJob::iconName(jobState), RoleJobIconName);
        colStatus->setData(KCupsJob::cancelEnabled(jobState), RoleJobCancelEnabled);
        colStatus->setData(KCupsJob::holdEnabled(jobState), RoleJobHoldEnabled);
        colStatus->setData(KCupsJob::releaseEnabled(jobState), RoleJobReleaseEnabled);
        colStatus->setData(job.reprintEnabled(), RoleJobRestartEnabled);
    }

    const QString pages =
        job.processedPages() ? QString::number(job.processedPages()) + QLatin1Char('/') + QString::number(job.processedPages()) : QString::number(job.pages());
    if (colStatus->data(RoleJobPages) != pages) {
        colStatus->setData(pages, RoleJobPages);
    }

    colStatus->setData(job.authenticationRequired(), RoleJobAuthenticationRequired);

    // internal dest name & column
    const QString destName = job.printer();
    if (colStatus->data(RoleJobPrinter).toString() != destName) {
        colStatus->setData(destName, RoleJobPrinter);
        // Column job printer Name
        item(pos, ColPrinter)->setText(destName);
    }

    // job name
    const QString jobName = job.name();
    if (item(pos, ColName)->text() != jobName) {
        colStatus->setData(jobName, RoleJobName);
        item(pos, ColName)->setText(jobName);
    }

    // owner of the job
    // try to get the full user name
    QString userString = job.originatingUserName();
    const KUser user(userString);
    if (user.isValid() && !user.property(KUser::FullName).toString().isEmpty()) {
        userString = user.property(KUser::FullName).toString();
    }

    // user name
    QStandardItem *colUser = item(pos, ColUser);
    if (colUser->text() != userString) {
        colUser->setText(userString);
    }

    // when it was created
    const auto timeAtCreation = QLocale().toString(job.createdAt(), QStringLiteral("ddd MMMM d yyyy"));
    QStandardItem *colCreated = item(pos, ColCreated);
    if (colCreated->data(Qt::DisplayRole) != timeAtCreation) {
        colCreated->setData(timeAtCreation, Qt::DisplayRole);
    }

    // when it was completed
    const auto completedAt = QLocale().toString(job.completedAt(), QStringLiteral("ddd MMMM d yyyy"));
    QStandardItem *colCompleted = item(pos, ColCompleted);
    if (colCompleted->data(Qt::DisplayRole) != completedAt) {
        if (!completedAt.isNull()) {
            colCompleted->setData(completedAt, Qt::DisplayRole);
        } else {
            // Clean the data might happen when the job is restarted
            colCompleted->setText(QString());
        }
    }

    // job pages
    const int completedPages = job.processedPages();
    QStandardItem *colPages = item(pos, ColPages);
    if (colPages->data(Qt::UserRole) != completedPages) {
        colPages->setData(completedPages, Qt::UserRole);
        colPages->setText(QString::number(completedPages));
    }

    // when it was processed
    const auto timeAtProcessing = QLocale().toString(job.processedAt(), QStringLiteral("ddd MMMM d yyyy"));
    QStandardItem *colProcessed = item(pos, ColProcessed);
    if (colProcessed->data(Qt::DisplayRole) != timeAtProcessing) {
        if (!timeAtProcessing.isNull()) {
            colProcessed->setData(timeAtProcessing, Qt::DisplayRole);
        } else {
            // Clean the data might happen when the job is restarted
            colCompleted->setText(QString());
        }
    }

    int jobSize = job.size();
    QStandardItem *colSize = item(pos, ColSize);
    if (colSize->data(Qt::UserRole) != jobSize) {
        colSize->setData(jobSize, Qt::UserRole);
        colSize->setText(KFormat().formatByteSize(jobSize));
    }

    // job printer state message
    const QString stateMessage = job.stateMsg();
    QStandardItem *colStatusMessage = item(pos, ColStatusMessage);
    if (colStatusMessage->text() != stateMessage) {
        colStatusMessage->setText(stateMessage);
    }

    // owner of the job
    // try to get the full user name
    const QString originatingHostName = job.originatingHostName();
    QStandardItem *colFromHost = item(pos, ColFromHost);
    if (colFromHost->text() != originatingHostName) {
        colFromHost->setText(originatingHostName);
    }
}

QHash<int, QByteArray> JobModel::roleNames() const
{
    return m_roles;
}

int JobModel::jobRow(int jobId) const
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (jobId == item(i)->data(RoleJobId).toInt()) {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString JobModel::jobStatus(ipp_jstate_e job_state) const
{
    const static QHash<int, QString> statusStrings({{IPP_JOB_PENDING, i18n("Pending")},
                                            {IPP_JOB_HELD, i18n("On hold")},
                                            {IPP_JOB_PROCESSING, QLatin1String("-")},
                                            {IPP_JOB_STOPPED, i18n("Stopped")},
                                            {IPP_JOB_CANCELED, i18n("Canceled")},
                                            {IPP_JOB_ABORTED, i18n("Aborted")},
                                            {IPP_JOB_COMPLETED, i18n("Completed")}
                                           });

    const auto str = statusStrings.value(job_state);
    return !str.isEmpty() ? str : QLatin1String("-");
}

void JobModel::clear()
{
    removeRows(0, rowCount());
}

void JobModel::setJobFilter(WhichJobs filter)
{
    m_jobFilter = filter;
    Q_EMIT jobFilterChanged();
    getJobs();
}

QStringList JobModel::messages() const
{
    return m_messages;
}

JobModel::WhichJobs JobModel::jobFilter() const
{
    return m_jobFilter;
}

#include "moc_JobModel.cpp"
