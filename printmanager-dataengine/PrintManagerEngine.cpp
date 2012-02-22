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

#include <KCupsRequest.h>
#include <KCupsJob.h>
 
PrintManagerEngine::PrintManagerEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)

    // This prevents applets from setting an unnecessarily high
    // update interval and using too much CPU.
    // In the case of a clock that only has second precision,
    // a third of a second should be more than enough.
    setMinimumPollingInterval(800);
}

void PrintManagerEngine::init()
{
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
}

Plasma::Service* PrintManagerEngine::serviceForSource(const QString &source)
{
    kDebug() << source;
    QStringList parts = source.split(QLatin1Char('/'));
    if (parts.size() == 2 || parts.size() == 4) {
        // Options
        // Printers / printer_name == 2
        // Or
        // Printers / printer_name / WhichJobs / job_id == 4
        // Or
        // WhichJobs / job_id == 2
        if (parts.first().startsWith(QLatin1String("Printers")) && parts.size() == 2) {
            return new PrintManagerService(this, parts.at(1));
        } else {

            return new PrintManagerService(this, parts.last().toInt());
        }
    }
    return Plasma::DataEngine::serviceForSource(source);
}


void PrintManagerEngine::job(const QString &prefix, int order, const KCupsJob &job)
{
    QString id = QString::number(job.id());
    id.prepend(prefix + QLatin1Char('/'));

    Data sourceData = query(id);
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
        pages = QString::number(job.processedPages());
        pages.append(QLatin1Char('/'));
        pages.append(QString::number(job.processedPages()));
        if (sourceData[QLatin1String("jobPages")] != pages) {
            sourceData[QLatin1String("jobPages")] = pages;
            changed = true;
        }
    }
//    setData(id, QLatin1String("jobPrinterStatMessage"), job.name());

    if (changed) {
        // update only if data changes to avoid uneeded updates on the views
        setData(id, sourceData);
    }
}

void PrintManagerEngine::updateJobs(const QString &prefix, const KCupsRequest::KCupsJobs &jobs)
{
    QStringList jobsStrList;
    for (int i = 0; i < jobs.size(); ++i) {
        job(prefix, i, jobs.at(i));
        jobsStrList << jobs.at(i).idStr();
    }

    // this RegExp matches all sources that start with 'prefix'
    // and are not followed by one of the printers names
    QRegExp rx(QLatin1Char('^') + prefix + QLatin1String("/(?!") + jobsStrList.join(QLatin1String("|")) + QLatin1Char(')'));

    foreach (const QString &source, sources().filter(rx)) {
        // Remove these as their printers or jobs are not available anymore
        removeSource(source);
    }
}

void PrintManagerEngine::requestJobsFinished()
{
    // TODO remove invalid sources
}

void PrintManagerEngine::printer(const QString &prefix, int order, const KCupsPrinter &printer)
{
    QString name = printer.name();
    name.prepend(prefix + QLatin1Char('/'));

    Data sourceData = query(name);
    bool changed = false;

    if (sourceData[QLatin1String("order")] != order) {
        sourceData[QLatin1String("order")] = order;
        changed = true;
    }
    if (sourceData[QLatin1String("printerName")] != printer.name()) {
        sourceData[QLatin1String("printerName")] = printer.name();
        changed = true;
    }
    if (sourceData[QLatin1String("info")] != printer.info()) {
        sourceData[QLatin1String("info")] = printer.info();
        changed = true;
    }
    QString state;
    switch (printer.state()) {
    case KCupsPrinter::Idle:
        state = QLatin1String("idle");
        break;
    case KCupsPrinter::Printing:
        state = QLatin1String("printing");
        break;
    case KCupsPrinter::Stoped:
        state = QLatin1String("stopped");
        break;
    default:
        state = QLatin1String("unknown");
    }
    if (sourceData[QLatin1String("stateEnum")] != state) {
        sourceData[QLatin1String("stateEnum")] = state;
        changed = true;
    }
    if (sourceData[QLatin1String("stateMessage")] != printer.stateMsg()) {
        sourceData[QLatin1String("stateMessage")] = printer.stateMsg();
        changed = true;
    }
    if (sourceData[QLatin1String("iconName")] != printer.iconName()) {
        sourceData[QLatin1String("iconName")] = printer.iconName();
        changed = true;
    }

    if (changed) {
        // update only if data changes to avoid uneeded updates on the views
        setData(name, sourceData);
    }
}

void PrintManagerEngine::updatePrinters(const QString &prefix, const KCupsRequest::KCupsPrinters &printers)
{
    QStringList printersStrList;
    for (int i = 0; i < printers.size(); ++i) {
        printer(prefix, i, printers.at(i));
        printersStrList << printers.at(i).name();
    }

    // this RegExp matches all sources that start with 'Printer/'
    // and are not followed by one of the printers names
    QRegExp rx(QLatin1String("^Printers/(?!") + printersStrList.join(QLatin1String("|")) + QLatin1Char(')'));

    foreach (const QString &source, sources().filter(rx)) {
        // Remove these as their printers are not available anymore
        removeSource(source);
    }
}

void PrintManagerEngine::requestPrintersFinished()
{
    // TODO remove invalid sources
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    request->deleteLater();
}
 
bool PrintManagerEngine::sourceRequestEvent(const QString &name)
{
    kDebug() << name;
    QRegExp rx(QLatin1String("^(?:AllJobs|ActiveJobs|CompletedJobs)|Printers(?:/[^/]+/(?:AllJobs|ActiveJobs|CompletedJobs))?$"));
    if (rx.exactMatch(name)) {
        m_validSources << name;
        // Needed so that DataSource can do polling
        // on e.g. "Printers" or "ActiveJobs"
        setData(name, Data());
        // We do not have any special code to execute the
        // first time a source is requested, so we just call
        // updateSourceEvent().

        return updateSourceEvent(name);
    }
    return false;
}
 
bool PrintManagerEngine::updateSourceEvent(const QString &name)
{ 
    KCupsRequest *request = new KCupsRequest;

    if (!m_validSources.contains(name)) {
        return false;
    }

    kDebug() << "updating" << name;

    if (name.startsWith(QLatin1String("Printers")) && name.count(QLatin1Char('/')) < 2) {
            KCupsPrinter::Attributes attr;
            attr |= KCupsPrinter::PrinterName;
            attr |= KCupsPrinter::PrinterInfo;
            attr |= KCupsPrinter::PrinterState;
            attr |= KCupsPrinter::PrinterStateMessage;
            // to get proper icons
            attr |= KCupsPrinter::PrinterType;
            request->getPrinters(attr);
            request->waitTillFinished();
            updatePrinters(QLatin1String("Printers"), request->printers());
    } else {
        QString printer;
        QString whichJob;
        QStringList parts = name.split(QLatin1Char('/'));
        if (parts.size() == 3) {
            // Printers PrinterName Kind of the job
            printer = parts.at(1);
            whichJob = parts.at(2);
        } else {
            whichJob = parts.at(0);
        }

        if (whichJob == QLatin1String("AllJobs")) {
            request->getJobs(printer, false, CUPS_WHICHJOBS_ALL, m_jobAttributes);
        } else if (whichJob == QLatin1String("ActiveJobs")) {
            request->getJobs(printer, false, CUPS_WHICHJOBS_ACTIVE, m_jobAttributes);
        } else if (whichJob == QLatin1String("CompletedJobs")) {
            request->getJobs(printer, false, CUPS_WHICHJOBS_COMPLETED, m_jobAttributes);
        } else {
            request->deleteLater();
            return false;
        }

        request->waitTillFinished();
        updateJobs(name, request->jobs());
    }

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
