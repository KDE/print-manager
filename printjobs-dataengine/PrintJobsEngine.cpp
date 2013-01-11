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
 
#include "PrintJobsEngine.h"

#include "PrintJobsService.h"
  
#include <Plasma/DataContainer>
#include <QStringBuilder>

#include <QDBusConnection>

#include <KCupsRequest.h>
#include <KCupsJob.h>
 
PrintJobsEngine::PrintJobsEngine(QObject *parent, const QVariantList &args) :
    Plasma::DataEngine(parent, args)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)

    KGlobal::insertCatalog(QLatin1String("print-manager"));

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
}

PrintJobsEngine::~PrintJobsEngine()
{
}

void PrintJobsEngine::init()
{
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

    // Deprecated stuff that works better than the above
    connect(KCupsConnection::global(), SIGNAL(rhQueueChanged(QString)),
            this, SLOT(getJobs()));
    connect(KCupsConnection::global(), SIGNAL(rhJobQueuedLocal(QString,uint,QString)),
            this, SLOT(insertUpdateJob(QString,uint,QString)));

    // Get all jobs
    getJobs();
}

Plasma::Service* PrintJobsEngine::serviceForSource(const QString &source)
{
    return new PrintJobsService(this, source);
}

void PrintJobsEngine::getJobs()
{
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(finished()), this, SLOT(getJobsFinished()));
    request->getJobs(QString(), false, CUPS_WHICHJOBS_ACTIVE, m_jobAttributes);
}

void PrintJobsEngine::getJobsFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    if (!request || request->hasError()) {
        // in case of an error probe the server again in 1.5 seconds
        QTimer::singleShot(1500, this, SLOT(getJobs()));
        request->deleteLater();;
        return;
    }

    QStringList jobsStrList;
    foreach (const KCupsJob &job, request->jobs()) {
        updateJobSource(job);
        jobsStrList << job.idStr();
    }

    // Remove the printers that are not available anymore
    foreach (const QString &source, sources()) {
        if (!jobsStrList.contains(source)) {
            removeSource(source);
        }
    }

    request->deleteLater();
}

void PrintJobsEngine::jobCompleted(const QString &text,
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
    Q_UNUSED(jobState)
    Q_UNUSED(jobStateReasons)
    Q_UNUSED(jobName)
    Q_UNUSED(jobImpressionsCompleted)

    // Remove the job source
    removeSource(QString::number(jobId));
}

void PrintJobsEngine::insertUpdateJob(uint jobId, const QString &printerUri)
{
    KCupsRequest *request = new KCupsRequest;
    // TODO we set is class to false, but what if it was a class?
    request->getJobAttributes(jobId, printerUri, m_jobAttributes);
    connect(request, SIGNAL(finished()), this, SLOT(insertUpdateJobFinished()));
}

void PrintJobsEngine::insertUpdateJob(const QString &queueName, uint jobId, const QString &jobOwner)
{
    Q_UNUSED(jobOwner)
    // TODO this is not the printer URI
    insertUpdateJob(jobId, queueName);
}

void PrintJobsEngine::insertUpdateJob(const QString &text,
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
    Q_UNUSED(printerName)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)
    Q_UNUSED(jobState)
    Q_UNUSED(jobStateReasons)
    Q_UNUSED(jobName)
    Q_UNUSED(jobImpressionsCompleted)

    kDebug() << jobId << jobState << jobStateReasons << jobName << jobImpressionsCompleted;

    QString source = QString::number(jobId);
    Data sourceData = query(source);
    if (!sourceData.isEmpty()) {
        bool changed = false;

        if (sourceData[QLatin1String("jobName")] != jobName) {
            sourceData[QLatin1String("jobName")] = jobName;
            changed = true;
        }

        if (sourceData[QLatin1String("jobPrinter")] != printerName) {
            sourceData[QLatin1String("jobPrinter")] = printerName;
            changed = true;
        }

        if (updateJobState(sourceData, static_cast<ipp_jstate_t>(jobState))) {
            changed = true;
        }

        if (changed) {
            // update only if data changes to avoid uneeded updates on the views
            setData(source, sourceData);
        }
    } else {
        insertUpdateJob(jobId, printerUri);
    }
}

void PrintJobsEngine::insertUpdateJobFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    if (!request) {
        return;
    }
    if (request->hasError() || request->jobs().isEmpty()) {
        // In case of an error force an update of all printers
        getJobs();
    } else {
        // Add/Update our printer
        foreach (const KCupsJob &job, request->jobs()) {
            updateJobSource(job);
        }
    }

    request->deleteLater();
}

void PrintJobsEngine::updateJobSource(const KCupsJob &job)
{
    Data sourceData = query(job.idStr());
    bool changed = sourceData.isEmpty();

    sourceData[QLatin1String("jobId")] = job.id();
    sourceData[QLatin1String("jobName")] = job.name();

    if (changed) {
        QString size = KGlobal::locale()->formatByteSize(job.size());
        sourceData[QLatin1String("jobSize")] = size;

        QString createdAt = KGlobal::locale()->formatDateTime(job.createdAt());
        sourceData[QLatin1String("jobCreatedAt")] = createdAt;

        sourceData[QLatin1String("jobOwner")] = job.ownerName();
    }

    // the job printer name might change if the job is moved
    if (sourceData[QLatin1String("jobPrinter")] != job.printer()) {
        sourceData[QLatin1String("jobPrinter")] = job.printer();
        changed = true;
    }

    // this handle cancel, hold, release states
    if (updateJobState(sourceData, job.state())) {
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
        setData(job.idStr(), sourceData);
    }
}

bool PrintJobsEngine::updateJobState(Plasma::DataEngine::Data &sourceData, ipp_jstate_t jobState)
{
    bool changed = false;
    if (sourceData[QLatin1String("jobIconName")] != KCupsJob::iconName(jobState)) {
        sourceData[QLatin1String("jobIconName")] = KCupsJob::iconName(jobState);
        changed = true;
    }
    if (sourceData[QLatin1String("jobCancelEnabled")] != KCupsJob::cancelEnabled(jobState)) {
        sourceData[QLatin1String("jobCancelEnabled")] = KCupsJob::cancelEnabled(jobState);
        changed = true;
    }
    if (sourceData[QLatin1String("jobHoldEnabled")] != KCupsJob::holdEnabled(jobState)) {
        sourceData[QLatin1String("jobHoldEnabled")] = KCupsJob::holdEnabled(jobState);
        changed = true;
    }
    if (sourceData[QLatin1String("jobReleaseEnabled")] != KCupsJob::releaseEnabled(jobState)) {
        sourceData[QLatin1String("jobReleaseEnabled")] = KCupsJob::releaseEnabled(jobState);
        changed = true;
    }
    if (sourceData[QLatin1String("jobRestartEnabled")] != KCupsJob::restartEnabled(jobState)) {
        sourceData[QLatin1String("jobRestartEnabled")] = KCupsJob::restartEnabled(jobState);
        changed = true;
    }
    return changed;
}
 
// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// The second argument is the name of the class in
// your plugin that derives from Plasma::DataEngine
K_EXPORT_PLASMA_DATAENGINE(plasma_engine_printjobs, PrintJobsEngine)
 
// this is needed since PrintJobsEngine is a QObject
#include "PrintJobsEngine.moc"
