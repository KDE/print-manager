/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include "PrintQueueModel.h"

#include <KCupsRequest.h>
#include <KCupsPrinter.h>
#include <KCupsJob.h>

#include <QDateTime>
#include <QMimeData>
#include <QPointer>

#include <KUser>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>

PrintQueueModel::PrintQueueModel(const QString &destName, WId parentId, QObject *parent) :
    QStandardItemModel(parent),
    m_printer(new KCupsPrinter(destName)),
    m_jobRequest(0),
    m_destName(destName),
    m_whichjobs(CUPS_WHICHJOBS_ACTIVE),
    m_parentId(parentId)
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
    m_jobAttributes << KCUPS_JOB_MEDIA_PROGRESS;
    m_jobAttributes << KCUPS_JOB_MEDIA_SHEETS;
    m_jobAttributes << KCUPS_JOB_MEDIA_SHEETS_COMPLETED;
    m_jobAttributes << KCUPS_JOB_PRINTER_STATE_MESSAGE;
    m_jobAttributes << KCUPS_JOB_PRESERVED;

    // This is emitted when a job change it's state
    connect(KCupsConnection::global(),
            SIGNAL(jobState(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)),
            this,
            SLOT(insertUpdateJob(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // This is emitted when a job is created
    connect(KCupsConnection::global(),
            SIGNAL(jobCreated(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)),
            this,
            SLOT(insertUpdateJob(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // This is emitted when a job is stopped
    connect(KCupsConnection::global(),
            SIGNAL(jobStopped(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)),
            this,
            SLOT(insertUpdateJob(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // This is emitted when a job has it's config changed
    connect(KCupsConnection::global(),
            SIGNAL(jobConfigChanged(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)),
            this,
            SLOT(insertUpdateJob(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // This is emitted when a job change it's progress
    connect(KCupsConnection::global(),
            SIGNAL(jobProgress(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)),
            this,
            SLOT(insertUpdateJob(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(),
            SIGNAL(jobCompleted(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)),
            this,
            SLOT(jobCompleted(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // Get all jobs
    getJobs();
}

void PrintQueueModel::getJobs()
{
    if (m_jobRequest) {
        return;
    }

    m_jobRequest = new KCupsRequest;
    connect(m_jobRequest, SIGNAL(finished()), this, SLOT(getJobFinished()));

    m_jobRequest->getJobs(m_destName, false, m_whichjobs, m_jobAttributes);

    m_processingJob.clear();
}

void PrintQueueModel::getJobFinished()
{
    KCupsRequest *request = static_cast<KCupsRequest *>(sender());
    if (request) {
        if (request->hasError()) {
//            emit error(request->error(), request->serverError(), request->errorMsg());
            // clear the model after so that the proper widget can be shown
//            clear();// TODO remove also in printerModel
        } else {
            KCupsJobs jobs = request->jobs();
            kDebug() << jobs.size();
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
        kWarning() << "Should not be called from a non KCupsRequest class" << sender();
    }
    m_jobRequest = 0;
}

void PrintQueueModel::jobCompleted(const QString &text,
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

void PrintQueueModel::insertUpdateJob(const QString &text,
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

void PrintQueueModel::insertJob(int pos, const KCupsJob &job)
{
    // insert the first column which has the job state and id
    QList<QStandardItem*> row;
    ipp_jstate_e jobState = job.state();
    QStandardItem *statusItem = new QStandardItem(jobStatus(jobState));
    statusItem->setData(jobState, JobState);
    statusItem->setData(job.id(), JobId);
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

void PrintQueueModel::updateJob(int pos, const KCupsJob &job)
{
    // Job Status & internal dataipp_jstate_e
    ipp_jstate_e jobState = job.state();
    if (item(pos, ColStatus)->data(JobState).toInt() != jobState) {
        item(pos, ColStatus)->setText(jobStatus(jobState));
        item(pos, ColStatus)->setData(static_cast<int>(jobState), JobState);
    }

    // internal dest name & column
    QString destName = job.printer();
    if (item(pos, ColStatus)->data(DestName).toString() != destName) {
        item(pos, ColStatus)->setData(destName, DestName);
        // Column job printer Name
        item(pos, ColPrinter)->setText(destName);
    }

    // job name
    QString jobName = job.name();
    if (item(pos, ColName)->text() != jobName) {
        item(pos, ColName)->setText(jobName);
    }

    // owner of the job
    // try to get the full user name
    QString userString = job.ownerName();
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

    // job size TODO use kde converter
    int jobSize = job.size();
    if (item(pos, ColSize)->data(Qt::UserRole) != jobSize) {
        item(pos, ColSize)->setData(jobSize, Qt::UserRole);
        item(pos, ColSize)->setText(KGlobal::locale()->formatByteSize(jobSize));
    }

    // job printer state message
    QString stateMessage = job.stateMsg();
    if (item(pos, ColStatusMessage)->text() != stateMessage) {
        item(pos, ColStatusMessage)->setText(stateMessage);
    }
}

QStringList PrintQueueModel::mimeTypes() const
{
    return QStringList("application/x-cupsjobs");
}

Qt::DropActions PrintQueueModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData* PrintQueueModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid() && index.column() == 0) {
            // serialize the jobId and fromDestName
            stream << data(index, JobId).toInt()
                   << data(index, DestName).toString()
                   << item(index.row(), ColName)->text();
        }
    }

    mimeData->setData("application/x-cupsjobs", encodedData);
    return mimeData;
}

bool PrintQueueModel::dropMimeData(const QMimeData *data,
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

    if (!data->hasFormat("application/x-cupsjobs")) {
        return false;
    }

    QByteArray encodedData = data->data("application/x-cupsjobs");
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

KCupsRequest* PrintQueueModel::modifyJob(int row, JobAction action, const QString &newDestName, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    QStandardItem *job = item(row, ColStatus);
    int jobId = job->data(JobId).toInt();
    QString destName = job->data(DestName).toString();

    // ignore some jobs
    ipp_jstate_t state = (ipp_jstate_t) job->data(PrintQueueModel::JobState).toInt();
    if ((state == IPP_JOB_HELD && action == Hold) ||
        (state == IPP_JOB_CANCELED && action == Cancel) ||
        (state != IPP_JOB_HELD && action == Release)) {
        return 0;
    }

    KCupsRequest *request = new KCupsRequest;
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
    case Move:
        request->moveJob(destName, jobId, newDestName);
        break;
    default:
        kWarning() << "Unknown ACTION called!!!" << action;
        return 0;
    }

    return request;
}

int PrintQueueModel::jobRow(int jobId)
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (jobId == item(i)->data(JobId).toInt())
        {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString PrintQueueModel::jobStatus(ipp_jstate_e job_state)
{
  switch (job_state)
  {
    case IPP_JOB_PENDING    : return i18n("Pending");
    case IPP_JOB_HELD       : return i18n("On hold");
    case IPP_JOB_PROCESSING : return "-";
    case IPP_JOB_STOPPED    : return i18n("Stopped");
    case IPP_JOB_CANCELED   : return i18n("Canceled");
    case IPP_JOB_ABORTED    : return i18n("Aborted");
    case IPP_JOB_COMPLETED  : return i18n("Completed");
  }
  return "-";
}

void PrintQueueModel::setWhichJobs(int whichjobs)
{
    m_whichjobs = whichjobs;
    getJobs();
}

Qt::ItemFlags PrintQueueModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        ipp_jstate_t state = static_cast<ipp_jstate_t>(item(index.row(), ColStatus)->data(JobState).toInt());
        if (state == IPP_JOB_PENDING ||
            state == IPP_JOB_PROCESSING) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

QString PrintQueueModel::processingJob() const
{
    return m_processingJob;
}

#include "PrintQueueModel.moc"
