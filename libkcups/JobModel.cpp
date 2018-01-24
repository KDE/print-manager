/***************************************************************************
 *   Copyright (C) 2010-2018 by Daniel Nicoletti <dantti12@gmail.com>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "JobModel.h"

#include <Debug.h>
#include <KCupsRequest.h>
#include <KCupsPrinter.h>
#include <KCupsJob.h>

#include <QDateTime>
#include <QMimeData>
#include <QPointer>

#include <KFormat>
#include <KUser>
#include <KLocalizedString>
#include <KMessageBox>

#include <QStringBuilder>

JobModel::JobModel(QObject *parent) :
    QStandardItemModel(parent),
    m_jobRequest(0),
    m_whichjobs(CUPS_WHICHJOBS_ACTIVE),
    m_parentId(0)
{
    setHorizontalHeaderItem(ColStatus,        new QStandardItem(i18n("Status")));
    setHorizontalHeaderItem(ColName,          new QStandardItem(i18n("Name")));
    setHorizontalHeaderItem(ColUser,          new QStandardItem(i18n("User")));
    setHorizontalHeaderItem(ColCreated,       new QStandardItem(i18n("Created")));
    setHorizontalHeaderItem(ColCompleted,     new QStandardItem(i18n("Completed")));
    setHorizontalHeaderItem(ColPages,         new QStandardItem(i18n("Pages")));
    setHorizontalHeaderItem(ColProcessed,     new QStandardItem(i18n("Processed")));
    setHorizontalHeaderItem(ColSize,          new QStandardItem(i18n("Size")));
    setHorizontalHeaderItem(ColStatusMessage, new QStandardItem(i18n("Status Message")));
    setHorizontalHeaderItem(ColPrinter,       new QStandardItem(i18n("Printer")));
    setHorizontalHeaderItem(ColFromHost,      new QStandardItem(i18n("From Hostname")));

    // Setup the attributes we want from jobs
    m_jobAttributes << KCUPS_JOB_ID;
    m_jobAttributes << KCUPS_JOB_NAME;
    m_jobAttributes << KCUPS_JOB_K_OCTETS;
    m_jobAttributes << KCUPS_JOB_K_OCTETS_PROCESSED;
    m_jobAttributes << KCUPS_JOB_STATE;
    m_jobAttributes << KCUPS_TIME_AT_COMPLETED;
    m_jobAttributes << KCUPS_TIME_AT_CREATION;
    m_jobAttributes << KCUPS_TIME_AT_PROCESSING;
    m_jobAttributes << KCUPS_JOB_PRINTER_URI;
    m_jobAttributes << KCUPS_JOB_ORIGINATING_USER_NAME;
    m_jobAttributes << KCUPS_JOB_ORIGINATING_HOST_NAME;
    m_jobAttributes << KCUPS_JOB_MEDIA_PROGRESS;
    m_jobAttributes << KCUPS_JOB_MEDIA_SHEETS;
    m_jobAttributes << KCUPS_JOB_MEDIA_SHEETS_COMPLETED;
    m_jobAttributes << KCUPS_JOB_PRINTER_STATE_MESSAGE;
    m_jobAttributes << KCUPS_JOB_PRESERVED;

    m_roles = QStandardItemModel::roleNames();
    m_roles[RoleJobId] = "jobId";
    m_roles[RoleJobState] = "jobState";
    m_roles[RoleJobName] = "jobName";
    m_roles[RoleJobPages] = "jobPages";
    m_roles[RoleJobSize] = "jobSize";
    m_roles[RoleJobOwner] = "jobOwner";
    m_roles[RoleJobCreatedAt] = "jobCreatedAt";
    m_roles[RoleJobIconName] = "jobIconName";
    m_roles[RoleJobCancelEnabled] = "jobCancelEnabled";
    m_roles[RoleJobHoldEnabled] = "jobHoldEnabled";
    m_roles[RoleJobReleaseEnabled] = "jobReleaseEnabled";
    m_roles[RoleJobRestartEnabled] = "jobRestartEnabled";
    m_roles[RoleJobPrinter] = "jobPrinter";
    m_roles[RoleJobOriginatingHostName] = "jobFrom";

    // This is emitted when a job change it's state
    connect(KCupsConnection::global(), &KCupsConnection::jobState, this, &JobModel::insertUpdateJob);

    // This is emitted when a job is created
    connect(KCupsConnection::global(), &KCupsConnection::jobCreated, this, &JobModel::insertUpdateJob);

    // This is emitted when a job is stopped
    connect(KCupsConnection::global(), &KCupsConnection::jobStopped, this, &JobModel::insertUpdateJob);

    // This is emitted when a job has it's config changed
    connect(KCupsConnection::global(), &KCupsConnection::jobConfigChanged, this, &JobModel::insertUpdateJob);

    // This is emitted when a job change it's progress
    connect(KCupsConnection::global(), &KCupsConnection::jobProgress, this, &JobModel::insertUpdateJob);

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(), &KCupsConnection::jobCompleted, this, &JobModel::jobCompleted);

    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &JobModel::getJobs);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &JobModel::getJobs);
}

void JobModel::setParentWId(WId parentId)
{
    m_parentId = parentId;
}

void JobModel::init(const QString &destName)
{
    m_destName = destName;

    // Get all jobs
    getJobs();
}

void JobModel::hold(const QString &printerName, int jobId)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->holdJob(printerName, jobId);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void JobModel::release(const QString &printerName, int jobId)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->releaseJob(printerName, jobId);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void JobModel::cancel(const QString &printerName, int jobId)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->cancelJob(printerName, jobId);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void JobModel::move(const QString &printerName, int jobId, const QString &toPrinterName)
{
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->moveJob(printerName, jobId, toPrinterName);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void JobModel::getJobs()
{
    if (m_jobRequest) {
        return;
    }

    m_jobRequest = new KCupsRequest;
    connect(m_jobRequest, &KCupsRequest::finished, this, &JobModel::getJobFinished);

    m_jobRequest->getJobs(m_destName, false, m_whichjobs, m_jobAttributes);

    m_processingJob.clear();
}

void JobModel::getJobFinished(KCupsRequest *request)
{
    if (request) {
        if (request->hasError()) {
            // clear the model after so that the proper widget can be shown
            clear();
        } else {
            KCupsJobs jobs = request->jobs();
            qCDebug(LIBKCUPS) << jobs.size();
            for (int i = 0; i < jobs.size(); ++i) {
                if (jobs.at(i).state() == IPP_JOB_PROCESSING) {
                    m_processingJob = jobs.at(i).name();
                }

                // try to find the job row
                int job_row = jobRow(jobs.at(i).id());
                if (job_row == -1) {
                    // not found, insert new one
                    insertJob(i, jobs.at(i));
                } else if (job_row == i) {
                    // update the job
                    updateJob(i, jobs.at(i));
                } else {
                    // found at wrong position
                    // take it and insert on the right position
                    QList<QStandardItem *> row = takeRow(job_row);
                    insertRow(i, row);
                    updateJob(i, jobs.at(i));
                }
            }

            // remove old printers
            // The above code starts from 0 and make sure
            // dest == modelIndex(x) and if it's not the
            // case it either inserts or moves it.
            // so any item > num_jobs can be safely deleted
            while (rowCount() > jobs.size()) {
                removeRow(rowCount() - 1);
            }
        }
        request->deleteLater();
    } else {
        qCWarning(LIBKCUPS) << "Should not be called from a non KCupsRequest class" << sender();
    }
    m_jobRequest = 0;
}

void JobModel::jobCompleted(const QString &text,
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
    // REALLY? all these parameters just to say foo was deleted??
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

    // We grab all jobs again
    getJobs();
}

void JobModel::insertUpdateJob(const QString &text,
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
    // REALLY? all these parameters just to say foo was created??
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

    // We grab all jobs again
    getJobs();
}

void JobModel::insertJob(int pos, const KCupsJob &job)
{
    // insert the first column which has the job state and id
    QList<QStandardItem*> row;
    ipp_jstate_e jobState = job.state();
    auto statusItem = new QStandardItem(jobStatus(jobState));
    statusItem->setData(jobState, RoleJobState);
    statusItem->setData(job.id(), RoleJobId);
    statusItem->setData(job.name(), RoleJobName);
    statusItem->setData(job.originatingUserName(), RoleJobOwner);
    statusItem->setData(job.originatingHostName(), RoleJobOriginatingHostName);
    QString size = KFormat().formatByteSize(job.size());
    statusItem->setData(size, RoleJobSize);
    QString createdAt = QLocale().toString(job.createdAt());
    statusItem->setData(createdAt, RoleJobCreatedAt);

    // TODO move the update code before the insert and reuse some code...
    statusItem->setData(KCupsJob::iconName(jobState), RoleJobIconName);
    statusItem->setData(KCupsJob::cancelEnabled(jobState), RoleJobCancelEnabled);
    statusItem->setData(KCupsJob::holdEnabled(jobState), RoleJobHoldEnabled);
    statusItem->setData(KCupsJob::releaseEnabled(jobState), RoleJobReleaseEnabled);
    statusItem->setData(job.reprintEnabled(), RoleJobRestartEnabled);

    QString pages = QString::number(job.pages());
    if (job.processedPages()) {
        pages = QString::number(job.processedPages()) % QLatin1Char('/') % QString::number(job.processedPages());
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

void JobModel::updateJob(int pos, const KCupsJob &job)
{
    // Job Status & internal dataipp_jstate_e
    ipp_jstate_e jobState = job.state();
    if (item(pos, ColStatus)->data(RoleJobState).toInt() != jobState) {
        item(pos, ColStatus)->setText(jobStatus(jobState));
        item(pos, ColStatus)->setData(static_cast<int>(jobState), RoleJobState);

        item(pos, ColStatus)->setData(KCupsJob::iconName(jobState), RoleJobIconName);
        item(pos, ColStatus)->setData(KCupsJob::cancelEnabled(jobState), RoleJobCancelEnabled);
        item(pos, ColStatus)->setData(KCupsJob::holdEnabled(jobState), RoleJobHoldEnabled);
        item(pos, ColStatus)->setData(KCupsJob::releaseEnabled(jobState), RoleJobReleaseEnabled);
        item(pos, ColStatus)->setData(job.reprintEnabled(), RoleJobRestartEnabled);
    }

    QString pages = QString::number(job.pages());
    if (job.processedPages()) {
        pages = QString::number(job.processedPages()) % QLatin1Char('/') % QString::number(job.processedPages());
    }
    if (item(pos, ColStatus)->data(RoleJobPages) != pages) {
        item(pos, ColStatus)->setData(pages, RoleJobPages);
    }

    // internal dest name & column
    QString destName = job.printer();
    if (item(pos, ColStatus)->data(RoleJobPrinter).toString() != destName) {
        item(pos, ColStatus)->setData(destName, RoleJobPrinter);
        // Column job printer Name
        item(pos, ColPrinter)->setText(destName);
    }

    // job name
    QString jobName = job.name();
    if (item(pos, ColName)->text() != jobName) {
        item(pos, ColStatus)->setData(jobName, RoleJobName);
        item(pos, ColName)->setText(jobName);
    }

    // owner of the job
    // try to get the full user name
    QString userString = job.originatingUserName();
    KUser user(userString);
    if (user.isValid() && !user.property(KUser::FullName).toString().isEmpty()) {
        userString = user.property(KUser::FullName).toString();
    }

    // user name
    if (item(pos, ColUser)->text() != userString) {
        item(pos, ColUser)->setText(userString);
    }

    // when it was created
    QDateTime timeAtCreation = job.createdAt();
    if (item(pos, ColCreated)->data(Qt::DisplayRole).toDateTime() != timeAtCreation) {
        item(pos, ColCreated)->setData(timeAtCreation, Qt::DisplayRole);
    }

    // when it was completed
    QDateTime completedAt = job.completedAt();
    if (item(pos, ColCompleted)->data(Qt::DisplayRole).toDateTime() != completedAt) {
        if (!completedAt.isNull()) {
            item(pos, ColCompleted)->setData(completedAt, Qt::DisplayRole);
        } else {
            // Clean the data might happen when the job is restarted
            item(pos, ColCompleted)->setText(QString());
        }
    }

    // job pages
    int completedPages = job.processedPages();
    if (item(pos, ColPages)->data(Qt::UserRole) != completedPages) {
        item(pos, ColPages)->setData(completedPages, Qt::UserRole);
        item(pos, ColPages)->setText(QString::number(completedPages));
    }

    // when it was precessed
    QDateTime timeAtProcessing = job.processedAt();
    if (item(pos, ColProcessed)->data(Qt::DisplayRole).toDateTime() != timeAtProcessing) {
        if (!timeAtProcessing.isNull()) {
            item(pos, ColProcessed)->setData(timeAtProcessing, Qt::DisplayRole);
        } else {
            // Clean the data might happen when the job is restarted
            item(pos, ColCompleted)->setText(QString());
        }
    }

    int jobSize = job.size();
    if (item(pos, ColSize)->data(Qt::UserRole) != jobSize) {
        item(pos, ColSize)->setData(jobSize, Qt::UserRole);
        item(pos, ColSize)->setText(KFormat().formatByteSize(jobSize));
    }

    // job printer state message
    QString stateMessage = job.stateMsg();
    if (item(pos, ColStatusMessage)->text() != stateMessage) {
        item(pos, ColStatusMessage)->setText(stateMessage);
    }

    // owner of the job
    // try to get the full user name
    QString originatingHostName = job.originatingHostName();
    if (item(pos, ColFromHost)->text() != originatingHostName) {
        item(pos, ColFromHost)->setText(originatingHostName);
    }
}

QStringList JobModel::mimeTypes() const
{
    return { QStringLiteral("application/x-cupsjobs") };
}

Qt::DropActions JobModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData* JobModel::mimeData(const QModelIndexList &indexes) const
{
    auto mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid() && index.column() == 0) {
            // serialize the jobId and fromDestName
            stream << data(index, RoleJobId).toInt()
                   << data(index, RoleJobPrinter).toString()
                   << item(index.row(), ColName)->text();
        }
    }

    mimeData->setData(QLatin1String("application/x-cupsjobs"), encodedData);
    return mimeData;
}

bool JobModel::dropMimeData(const QMimeData *data,
                                   Qt::DropAction action,
                                   int row,
                                   int column,
                                   const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!data->hasFormat(QLatin1String("application/x-cupsjobs"))) {
        return false;
    }

    QByteArray encodedData = data->data(QLatin1String("application/x-cupsjobs"));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    bool ret = false;
    while (!stream.atEnd()) {
        QString fromDestName, displayName;
        int jobId;
        // get the jobid and the from dest name
        stream >> jobId >> fromDestName >> displayName;
        if (fromDestName == m_destName) {
            continue;
        }

        QPointer<KCupsRequest> request = new KCupsRequest;
        request->moveJob(fromDestName, jobId, m_destName);
        request->waitTillFinished();
        if (request) {
            if (request->hasError()) {
                // failed to move one job
                // we return here to avoid more password tries
                KMessageBox::detailedSorryWId(m_parentId,
                                              i18n("Failed to move '%1' to '%2'",
                                                   displayName, m_destName),
                                              request->errorMsg(),
                                              i18n("Failed"));
            }
            request->deleteLater();
            ret = !request->hasError();
        }
    }
    return ret;
}

QHash<int, QByteArray> JobModel::roleNames() const
{
    return m_roles;
}

KCupsRequest* JobModel::modifyJob(int row, JobAction action, const QString &newDestName, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    if (row < 0 || row >= rowCount()) {
        qCWarning(LIBKCUPS) << "Row number is invalid:" << row;
        return 0;
    }

    QStandardItem *job = item(row, ColStatus);
    int jobId = job->data(RoleJobId).toInt();
    QString destName = job->data(RoleJobPrinter).toString();

    // ignore some jobs
    ipp_jstate_t state = static_cast<ipp_jstate_t>(job->data(RoleJobState).toInt());
    if ((state == IPP_JOB_HELD && action == Hold) ||
        (state == IPP_JOB_CANCELED && action == Cancel) ||
        (state != IPP_JOB_HELD && action == Release)) {
        return 0;
    }

    auto request = new KCupsRequest;
    switch (action) {
    case Cancel:
        request->cancelJob(destName, jobId);
        break;
    case Hold:
        request->holdJob(destName, jobId);
        break;
    case Release:
        request->releaseJob(destName, jobId);
        break;
    case Reprint:
        request->restartJob(destName, jobId);
        break;
    case Move:
        request->moveJob(destName, jobId, newDestName);
        break;
    default:
        qCWarning(LIBKCUPS) << "Unknown ACTION called!!!" << action;
        return 0;
    }

    return request;
}

int JobModel::jobRow(int jobId)
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (jobId == item(i)->data(RoleJobId).toInt())
        {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString JobModel::jobStatus(ipp_jstate_e job_state)
{
  switch (job_state)
  {
    case IPP_JOB_PENDING    : return i18n("Pending");
    case IPP_JOB_HELD       : return i18n("On hold");
    case IPP_JOB_PROCESSING : return QLatin1String("-");
    case IPP_JOB_STOPPED    : return i18n("Stopped");
    case IPP_JOB_CANCELED   : return i18n("Canceled");
    case IPP_JOB_ABORTED    : return i18n("Aborted");
    case IPP_JOB_COMPLETED  : return i18n("Completed");
  }
  return QLatin1String("-");
}

void JobModel::clear()
{
    removeRows(0, rowCount());
}

void JobModel::setWhichJobs(WhichJobs whichjobs)
{
    switch (whichjobs) {
    case WhichActive:
        m_whichjobs = CUPS_WHICHJOBS_ACTIVE;
        break;
    case WhichCompleted:
        m_whichjobs = CUPS_WHICHJOBS_COMPLETED;
        break;
    case WhichAll:
        m_whichjobs = CUPS_WHICHJOBS_ALL;
        break;
    }

    getJobs();
}

Qt::ItemFlags JobModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        ipp_jstate_t state = static_cast<ipp_jstate_t>(item(index.row(), ColStatus)->data(RoleJobState).toInt());
        if (state == IPP_JOB_PENDING ||
            state == IPP_JOB_PROCESSING) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

QString JobModel::processingJob() const
{
    return m_processingJob;
}
