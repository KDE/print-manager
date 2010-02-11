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

#include "QCups.h"

PrintQueueModel::PrintQueueModel(const QString &destName, WId parentId, QObject *parent)
 : QStandardItemModel(parent),
   m_destName(destName),
   m_whichjobs(CUPS_WHICHJOBS_ACTIVE),
   m_parentId(parentId)
{
    m_showPrinterColumn = destName.isNull();
}

void PrintQueueModel::updateModel()
{
    int num_jobs;
    cups_job_t *jobs;
    num_jobs = cupsGetJobs(&jobs, m_destName.toLocal8Bit().data(), 0, m_whichjobs);

    for (int i = 0; i < num_jobs; i++) {
        // try to find the job row
        int job_row = jobRow(jobs[i].id);
        if (job_row == -1) {
            // not found, insert new one
            insertJob(i, jobs[i]);
        } else if (job_row == i) {
            // update the job
            updateJob(i, jobs[i]);
        } else {
            // found at wrong position
            // take it and insert on the right position
            QList<QStandardItem *> row = takeRow(job_row);
            insertRow(i, row);
            updateJob(i, jobs[i]);
        }
    }

    // remove old jobs
    // The above code starts from 0 and make sure
    // jobs[x] == modelIndex(x) and if it's not the
    // case it either inserts or moves it.
    // so any item > num_jobs can be safely deleted
    while (rowCount() > num_jobs) {
        removeRow(rowCount() - 1);
    }

    // free the jobs list
    cupsFreeJobs(num_jobs, jobs);
}

void PrintQueueModel::insertJob(int pos, cups_job_s job)
{
    // insert the first column which has the job state and id
    QList<QStandardItem*> row;
    QStandardItem *stdItem = new QStandardItem(jobStatus(job.state));
    stdItem->setData(job.id, JobId);
    stdItem->setData(job.state, JobState);
    stdItem->setData(QString::fromLocal8Bit(job.dest), DestName);
    row << stdItem;

    // job name
    row << new QStandardItem(QString::fromLocal8Bit(job.title));

    // owner of the job
    // try to get the full user name
    KUser user(QString::fromLocal8Bit(job.user));
    if (user.isValid() && !user.property(KUser::FullName).toString().isEmpty()) {
        row << new QStandardItem(user.property(KUser::FullName).toString());
    } else {
        row << new QStandardItem(job.user);
    }

    // when it was created
    QDateTime creationTime;
    creationTime.setTime_t(job.creation_time);
    stdItem = new QStandardItem();
    stdItem->setData(creationTime, Qt::DisplayRole);
    row << stdItem;

    // when it was completed
    stdItem = new QStandardItem();
    if (job.completed_time != 0) {
        QDateTime completedTime;
        completedTime.setTime_t(job.completed_time);
        stdItem->setData(completedTime, Qt::DisplayRole);
    }
    row << stdItem;

    // if no printer queue is set we are search them all
    // so add a printer column
    if (m_showPrinterColumn) {
        row << new QStandardItem(QString::fromLocal8Bit(job.dest));
    }

    // insert the whole row
    insertRow(pos, row);
}

void PrintQueueModel::updateJob(int pos, cups_job_s job)
{
    item(pos)->setText(jobStatus(job.state));
    item(pos)->setData(job.state, JobState);
    item(pos)->setData(job.dest, DestName);
    if (job.completed_time != 0) {
        QDateTime completedTime;
        completedTime.setTime_t(job.completed_time);
        item(pos, ColCompleted)->setData(completedTime, Qt::DisplayRole);
    }

    if (m_showPrinterColumn) {
        item(pos, ColPrinter)->setText(QString::fromLocal8Bit(job.dest));
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

    foreach (const QModelIndex index, indexes) {
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

        if (!QCups::moveJob(fromDestName.toLocal8Bit().data(),
                            jobId,
                            m_destName.toLocal8Bit().data())) {
            // failed to move one job
            // we return here to avoid more password tries
            KMessageBox::detailedSorryWId(m_parentId,
                                          i18n("Failed to move '%1'",
                                               displayName),
                                          cupsLastErrorString(),
                                          i18n("Failed"));
            return false;
        }
        ret = true;
    }
    return ret;
}

bool PrintQueueModel::modifyJob(int row, JobAction action, const QModelIndex &parent)
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
        return true;
    }

    switch (action) {
    case Cancel:
        return QCups::cancelJob(destName.toLocal8Bit().data(), jobId);
    case Hold:
        return QCups::holdJob(destName.toLocal8Bit().data(), jobId);
    case Release:
        return QCups::releaseJob(destName.toLocal8Bit().data(), jobId);
    }
    return false;
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

QString PrintQueueModel::jobStatus(ipp_jstate_t job_state)
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

QVariant PrintQueueModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole) {
        switch(section) {
            case ColStatus:
                return i18n("Status");
            case ColName:
                return i18n("Name");
            case ColUser:
                return i18n("User");
            case ColCreated:
                return i18n("Created");
            case ColCompleted:
                return i18n("Completed");
            case ColPrinter:
                return i18n("Printer");
        }
    }
    return QVariant();
}

void PrintQueueModel::setWhichJobs(int whichjobs)
{
    m_whichjobs = whichjobs;
}

Qt::ItemFlags PrintQueueModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        ipp_jstate_t state = (ipp_jstate_t) item(index.row(), ColStatus)->data(JobState).toInt();
        if (state == IPP_JOB_PENDING ||
            state == IPP_JOB_PROCESSING) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
}

#include "PrintQueueModel.moc"
