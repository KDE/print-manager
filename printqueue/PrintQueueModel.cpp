/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#include <QDateTime>
#include <QMimeData>
#include <KUser>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>

PrintQueueModel::PrintQueueModel(const QString &destName, WId parentId, QObject *parent)
 : QStandardItemModel(parent),
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
    m_requestedAttr << "job-id"
                    << "job-name"
                    << "job-k-octets"
                    << "job-k-octets-processed"
                    << "job-state"
                    << "time-at-completed"
                    << "time-at-creation"
                    << "time-at-processing"
                    << "job-printer-uri"
                    << "job-originating-user-name"
                    << "job-media-progress"
                    << "job-media-sheets"
                    << "job-media-sheets-completed"
                    << "job-printer-state-message"
                    << "job-preserved";
}

void PrintQueueModel::updateModel()
{
    QCups::Result *ret = QCups::getJobs(m_destName, false, m_whichjobs, m_requestedAttr);
    ret->waitTillFinished();
    QCups::ReturnArguments jobs = ret->result();
    ret->deleteLater();

    m_processingJob.clear();
    for (int i = 0; i < jobs.size(); i++) {
        QCups::Arguments job = jobs.at(i);
        if (job["job-state"].toInt() == IPP_JOB_PROCESSING) {
              m_processingJob = job["job-name"].toString();
        }
        // try to find the job row
        int job_row = jobRow(job["job-id"].toInt());
        if (job_row == -1) {
            // not found, insert new one
            insertJob(i, job);
        } else if (job_row == i) {
            // update the job
            updateJob(i, job);
        } else {
            // found at wrong position
            // take it and insert on the right position
            QList<QStandardItem *> row = takeRow(job_row);
            insertRow(i, row);
            updateJob(i, job);
        }
    }

    // remove old jobs
    // The above code starts from 0 and make sure
    // jobs[x] == modelIndex(x) and if it's not the
    // case it either inserts or moves it.
    // so any item > num_jobs can be safely deleted
    while (rowCount() > jobs.size()) {
        removeRow(rowCount() - 1);
    }
}

void PrintQueueModel::insertJob(int pos, const QCups::Arguments &job)
{
    // insert the first column which has the job state and id
    QList<QStandardItem*> row;
    int jobState = job["job-state"].toInt();
    QStandardItem *statusItem = new QStandardItem(jobStatus(jobState));
    statusItem->setData(jobState, JobState);
    statusItem->setData(job["job-id"].toInt(), JobId);
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

void PrintQueueModel::updateJob(int pos, const QCups::Arguments &job)
{
    // Job Status & internal data
    int jobState = job["job-state"].toInt();
    if (item(pos, ColStatus)->data(JobState) != jobState) {
        item(pos, ColStatus)->setText(jobStatus(jobState));
        item(pos, ColStatus)->setData(jobState, JobState);
    }

    // internal dest name & column
    QString destName = job["job-printer-uri"].toString().section('/', -1);
    if (item(pos, ColStatus)->data(DestName).toString() != destName) {
        item(pos, ColStatus)->setData(destName, DestName);
        // Column job printer Name
        item(pos, ColPrinter)->setText(destName);
    }

    // job name
    QString jobName = job["job-name"].toString();
    if (item(pos, ColName)->text() != jobName) {
        item(pos, ColName)->setText(jobName);
    }

    // owner of the job
    // try to get the full user name
    QString userString = job["job-originating-user-name"].toString();
    KUser user(userString);
    if (user.isValid() && !user.property(KUser::FullName).toString().isEmpty()) {
        userString = user.property(KUser::FullName).toString();
    }

    // user name
    if (item(pos, ColUser)->text() != userString) {
        item(pos, ColUser)->setText(userString);
    }

    // when it was created
    int timeAtCreation = job["time-at-creation"].toInt();
    if (item(pos, ColCreated)->data(Qt::UserRole) != timeAtCreation) {
        QDateTime creationTime;
        creationTime.setTime_t(timeAtCreation);
        item(pos, ColCreated)->setData(creationTime, Qt::DisplayRole);
        item(pos, ColCreated)->setData(timeAtCreation, Qt::UserRole);
    }

    // when it was completed
    int timeAtCompleted = job["time-at-completed"].toInt();
    if (item(pos, ColCompleted)->data(Qt::UserRole) != timeAtCompleted) {
        if (timeAtCompleted != 0) {
            QDateTime completedTime;
            completedTime.setTime_t(timeAtCompleted);
            item(pos, ColCompleted)->setData(completedTime, Qt::DisplayRole);
            item(pos, ColCompleted)->setData(timeAtCompleted, Qt::UserRole);
        } else {
            // Clean the data might happen when the job is restarted
            item(pos, ColCompleted)->setText(QString());
            item(pos, ColCompleted)->setData(0, Qt::UserRole);
        }
    }

    // job pages
    int completedPages = job["job-media-sheets-completed"].toInt();
    if (item(pos, ColPages)->data(Qt::UserRole) != completedPages) {
        item(pos, ColPages)->setData(completedPages, Qt::UserRole);
        item(pos, ColPages)->setText(QString::number(completedPages));
    }

    // when it was precessed
    int timeAtProcessing = job["time-at-completed"].toInt();
    if (item(pos, ColProcessed)->data(Qt::UserRole) != timeAtProcessing) {
        if (timeAtCompleted != 0) {
            QDateTime precessedTime;
            precessedTime.setTime_t(timeAtProcessing);
            item(pos, ColProcessed)->setData(precessedTime, Qt::DisplayRole);
            item(pos, ColProcessed)->setData(timeAtProcessing, Qt::UserRole);
        } else {
            // Clean the data might happen when the job is restarted
            item(pos, ColCompleted)->setText(QString());
            item(pos, ColCompleted)->setData(0, Qt::UserRole);
        }
    }

    // job size TODO use kde converter
    int jobKOctets = job["job-k-octets"].toInt();
    if (item(pos, ColSize)->data(Qt::UserRole) != jobKOctets) {
        item(pos, ColSize)->setData(jobKOctets, Qt::UserRole);
        jobKOctets *= 1024; // transform it to bytes
        item(pos, ColSize)->setText(KGlobal::locale()->formatByteSize(jobKOctets));
    }

    // job printer state message
    QString stateMessage = job["job-printer-state-message"].toString();
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

        QCups::Result *result = QCups::moveJob(fromDestName, jobId, m_destName);
        result->waitTillFinished();
        result->deleteLater(); // TODO can it be deleted here?
        if (result->hasError()) {
            // failed to move one job
            // we return here to avoid more password tries
            KMessageBox::detailedSorryWId(m_parentId,
                                          i18n("Failed to move '%1' to '%2'",
                                               displayName, m_destName),
                                          result->lastErrorString(),
                                          i18n("Failed"));
            return false;
        }
        ret = true;
    }
    return ret;
}

QCups::Result* PrintQueueModel::modifyJob(int row, JobAction action, const QString &newDestName, const QModelIndex &parent)
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

    switch (action) {
    case Cancel:
        return QCups::cancelJob(destName, jobId);
    case Hold:
        return QCups::holdJob(destName, jobId);
    case Release:
        return QCups::releaseJob(destName, jobId);
    case Move:
        return QCups::moveJob(destName, jobId, newDestName);
    }
    kWarning() << "Action unknown!!";
    return 0;
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

QString PrintQueueModel::jobStatus(int job_state)
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
    updateModel();
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
