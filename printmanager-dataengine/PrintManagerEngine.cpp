/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti                                *
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
 
#include "PrintManagerEngine.h"

#include "PrintManagerService.h"
  
#include <Plasma/DataContainer>
#include <QStringBuilder>

#include <KCupsRequest.h>
#include <KCupsJob.h>
 
PrintManagerEngine::PrintManagerEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)

    // Setup the attributes we want from jobs
    m_jobAttributes |= KCupsJob::JobId;
    m_jobAttributes |= KCupsJob::JobName;
    m_jobAttributes |= KCupsJob::JobKOctets;
    m_jobAttributes |= KCupsJob::JobKOctetsProcessed;
    m_jobAttributes |= KCupsJob::JobState;
    m_jobAttributes |= KCupsJob::TimeAtCompleted;
    m_jobAttributes |= KCupsJob::TimeAtCreation;
    m_jobAttributes |= KCupsJob::TimeAtProcessing;
    m_jobAttributes |= KCupsJob::JobPrinterUri;
    m_jobAttributes |= KCupsJob::JobOriginatingUserName;
    m_jobAttributes |= KCupsJob::JobMediaProgress;
    m_jobAttributes |= KCupsJob::JobMediaSheets;
    m_jobAttributes |= KCupsJob::JobMediaSheetsCompleted;
    m_jobAttributes |= KCupsJob::JobPrinterStatMessage;
    m_jobAttributes |= KCupsJob::JobPreserved;

    // This prevents applets from setting an unnecessarily high
    // update interval and using too much CPU.
    // In the case of a clock that only has second precision,
    // a third of a second should be more than enough.
    setMinimumPollingInterval(800);
}

void PrintManagerEngine::init()
{
}

Plasma::Service* PrintManagerEngine::serviceForSource(const QString &source)
{
    QStringList parts = source.split(QLatin1Char('/'));
    if (parts.size() == 2) {
        // WhichJobs / job_id == 2
        return new PrintManagerService(this, parts.last());
    }
    return Plasma::DataEngine::serviceForSource(source);
}


void PrintManagerEngine::job(const QString &prefix, int order, const KCupsJob &job)
{
    QString source = prefix % job.idStr();

    Data sourceData = query(source);
    bool changed = false;

    if (sourceData[QLatin1String("order")] != order) {
        sourceData[QLatin1String("order")] = order;
        changed = true;
    }
    if (sourceData[QLatin1String("jobId")] != job.id()) {
        sourceData[QLatin1String("jobId")] = job.id();
        changed = true;
    }
    if (sourceData[QLatin1String("jobName")] != job.name()) {
        sourceData[QLatin1String("jobName")] = job.name();
        changed = true;
    }
    QString size = KGlobal::locale()->formatByteSize(job.size());
    if (sourceData[QLatin1String("jobSize")] != size) {
        sourceData[QLatin1String("jobSize")] = size;
        changed = true;
    }
    if (sourceData[QLatin1String("jobIconName")] != job.iconName()) {
        sourceData[QLatin1String("jobIconName")] = job.iconName();
        changed = true;
    }
    if (sourceData[QLatin1String("jobCompletedAt")] != job.completedAt()) {
        sourceData[QLatin1String("jobCompletedAt")] = job.completedAt();
        changed = true;
    }
    QString createdAt = KGlobal::locale()->formatDateTime(job.createdAt());
    if (sourceData[QLatin1String("jobCreatedAt")] != createdAt) {
        sourceData[QLatin1String("jobCreatedAt")] = createdAt;
        changed = true;
    }
    if (sourceData[QLatin1String("jobPrinter")] != job.printer()) {
        sourceData[QLatin1String("jobPrinter")] = job.printer();
        changed = true;
    }
    if (sourceData[QLatin1String("jobOwner")] != job.ownerName()) {
        sourceData[QLatin1String("jobOwner")] = job.ownerName();
        changed = true;
    }
    if (sourceData[QLatin1String("jobCancelEnabled")] != job.cancelEnabled()) {
        sourceData[QLatin1String("jobCancelEnabled")] = job.cancelEnabled();
        changed = true;
    }
    if (sourceData[QLatin1String("jobHoldEnabled")] != job.holdEnabled()) {
        sourceData[QLatin1String("jobHoldEnabled")] = job.holdEnabled();
        changed = true;
    }
    if (sourceData[QLatin1String("jobReleaseEnabled")] != job.releaseEnabled()) {
        sourceData[QLatin1String("jobReleaseEnabled")] = job.releaseEnabled();
        changed = true;
    }
    if (job.processedPages() == 0) {
        if (sourceData[QLatin1String("jobPages")] != job.pages()) {
            sourceData[QLatin1String("jobPages")] = job.pages();
            changed = true;
        }
    } else {
        QString pages;
        pages = QString::number(job.processedPages()) % QLatin1Char('/') % QString::number(job.processedPages());
        if (sourceData[QLatin1String("jobPages")] != pages) {
            sourceData[QLatin1String("jobPages")] = pages;
            changed = true;
        }
    }

    if (changed) {
        // update only if data changes to avoid uneeded updates on the views
        setData(source, sourceData);
    }
}

void PrintManagerEngine::updateJobs(const QString &prefix, const KCupsJobs &jobs)
{
    QStringList jobsStrList;
    for (int i = 0; i < jobs.size(); ++i) {
        job(prefix, i, jobs.at(i));
        jobsStrList << jobs.at(i).idStr();
    }

    // this RegExp matches all sources that start with 'prefix'
    // and are not followed by one of the printers names
    QRegExp rx;
    if (jobsStrList.isEmpty()) {
        // we don't have any jobs remove all sources that start with our prefix
        rx.setPattern(QLatin1Char('^') % prefix);
    } else {
        rx.setPattern(QLatin1Char('^') % prefix % QLatin1String("(?!") % jobsStrList.join(QLatin1String("|")) % QLatin1Char(')'));
    }

    foreach (const QString &source, sources().filter(rx)) {
        // Remove these as their printers or jobs are not available anymore
        removeSource(source);
    }
}

bool PrintManagerEngine::sourceRequestEvent(const QString &source)
{
    if (source == QLatin1String("ActiveJobs") ||
        source == QLatin1String("AllJobs") ||
        source == QLatin1String("CompletedJobs")) {
        // Needed so that DataSource can do polling
        // on e.g. "AllJobs" or "ActiveJobs"
        setData(source, Data());
        // We do not have any special code to execute the
        // first time a source is requested, so we just call
        // updateSourceEvent().

        return updateSourceEvent(source);
    }
    return false;
}
 
bool PrintManagerEngine::updateSourceEvent(const QString &source)
{ 
    if (source != QLatin1String("ActiveJobs") &&
        source != QLatin1String("AllJobs") &&
        source != QLatin1String("CompletedJobs")) {
        return false;
    }

    KCupsRequest *request = new KCupsRequest;
    if (source == QLatin1String("ActiveJobs")) {
        request->getJobs(QString(), false, CUPS_WHICHJOBS_ACTIVE, m_jobAttributes);
    } else if (source == QLatin1String("AllJobs")) {
        request->getJobs(QString(), false, CUPS_WHICHJOBS_ALL, m_jobAttributes);
    } else {
        request->getJobs(QString(), false, CUPS_WHICHJOBS_COMPLETED, m_jobAttributes);
    }
    request->waitTillFinished();
    updateJobs(source % QLatin1Char('/'), request->jobs());
    request->deleteLater();

    return true;
}
 
// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// The second argument is the name of the class in
// your plugin that derives from Plasma::DataEngine
K_EXPORT_PLASMA_DATAENGINE(testtime, PrintManagerEngine)
 
// this is needed since PrintManagerEngine is a QObject
#include "PrintManagerEngine.moc"
