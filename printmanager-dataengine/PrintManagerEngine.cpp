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
    setMinimumPollingInterval(1000);
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

    m_jobsRequest = new KCupsRequest;
    connect(m_jobsRequest, SIGNAL(job(int,KCupsJob)), this, SLOT(job(int,KCupsJob)));
    connect(m_jobsRequest, SIGNAL(finished()), this, SLOT(requestJobsFinished()));

    m_printersRequest = new KCupsRequest;
    connect(m_printersRequest, SIGNAL(printer(int,KCupsPrinter)), this, SLOT(printer(int,KCupsPrinter)));
    connect(m_printersRequest, SIGNAL(finished()), this, SLOT(requestPrintersFinished()));
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
        if (parts.first().startsWith(I18N_NOOP("Printers")) && parts.size() == 2) {
            return new PrintManagerService(this, parts.at(1));
        } else {
            return new PrintManagerService(this);
        }
    }
    return Plasma::DataEngine::serviceForSource(source);
}


void PrintManagerEngine::job(int order, const KCupsJob &job)
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    QString id = QString::number(job.id());
    id.prepend(request->property("prefix").toString() + QLatin1Char('/'));

    Data sourceData = query(id);
    bool changed = false;
    if (sourceData[I18N_NOOP("order")] != order) {
        sourceData[I18N_NOOP("order")] = order;
        changed = true;
    }
    if (sourceData[I18N_NOOP("jobId")] != job.id()) {
        sourceData[I18N_NOOP("jobId")] = job.id();
        changed = true;
    }
    if (sourceData[I18N_NOOP("jobName")] != job.name()) {
        sourceData[I18N_NOOP("jobName")] = job.name();
        changed = true;
    }
    if (sourceData[I18N_NOOP("jobSize")] != job.size()) {
        sourceData[I18N_NOOP("jobSize")] = job.size();
        changed = true;
    }
    QString jobState;
    switch (job.state()){
    case IPP_JOB_PENDING:
        jobState = QLatin1String("pending");
        break;
    case IPP_JOB_HELD:
        jobState = QLatin1String("on-hold");
        break;
    case IPP_JOB_PROCESSING:
        jobState = QLatin1String("-");
        break;
    case IPP_JOB_STOPPED:
        jobState = QLatin1String("stopped");
        break;
    case IPP_JOB_CANCELED:
        jobState = QLatin1String("canceled");
        break;
    case IPP_JOB_ABORTED:
        jobState = QLatin1String("aborted");
        break;
    case IPP_JOB_COMPLETED:
        jobState = QLatin1String("completed");
        break;
    default:
        jobState = QLatin1String("unknown");
    }
    if (sourceData[I18N_NOOP("jobState")] != jobState) {
        sourceData[I18N_NOOP("jobState")] = jobState;
        changed = true;
    }
    if (sourceData[I18N_NOOP("jobCompletedAt")] != job.completedAt()) {
        sourceData[I18N_NOOP("jobCompletedAt")] = job.completedAt();
        changed = true;
    }
    if (sourceData[I18N_NOOP("jobCreatedAt")] != job.createdAt()) {
        sourceData[I18N_NOOP("jobCreatedAt")] = job.createdAt();
        changed = true;
    }
    if (sourceData[I18N_NOOP("jobPrinter")] != job.printer()) {
        sourceData[I18N_NOOP("jobPrinter")] = job.printer();
        changed = true;
    }
    if (sourceData[I18N_NOOP("jobOwner")] != job.ownerName()) {
        sourceData[I18N_NOOP("jobOwner")] = job.ownerName();
        changed = true;
    }
    if (job.processedPages() == 0) {
        if (sourceData[I18N_NOOP("jobPages")] != job.pages()) {
            sourceData[I18N_NOOP("jobPages")] = job.pages();
            changed = true;
        }
    } else {
        QString pages;
        pages = QString::number(job.processedPages());
        pages.append(QLatin1Char('/'));
        pages.append(QString::number(job.processedPages()));
        if (sourceData[I18N_NOOP("jobPages")] != pages) {
            sourceData[I18N_NOOP("jobPages")] = pages;
            changed = true;
        }
    }
//    setData(id, I18N_NOOP("jobPrinterStatMessage"), job.name());

    if (changed) {
        // update only if data changes to avoid uneeded updates on the views
        setData(id, sourceData);
    }
}

void PrintManagerEngine::requestJobsFinished()
{
    // TODO remove invalid sources
}

void PrintManagerEngine::printer(int order, const KCupsPrinter &printer)
{
    setData("Printers", Data());
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    QString name = printer.name();
    name.prepend(request->property("prefix").toString() + QLatin1Char('/'));

    Data sourceData = query(name);
    bool changed = false;

    if (sourceData[I18N_NOOP("order")] != order) {
        sourceData[I18N_NOOP("order")] = order;
        changed = true;
    }
    if (sourceData[I18N_NOOP("printerName")] != printer.name()) {
        sourceData[I18N_NOOP("printerName")] = printer.name();
        changed = true;
    }
    if (sourceData[I18N_NOOP("info")] != printer.info()) {
        sourceData[I18N_NOOP("info")] = printer.info();
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
    if (sourceData[I18N_NOOP("stateEnum")] != state) {
        sourceData[I18N_NOOP("stateEnum")] = state;
        changed = true;
    }
    if (sourceData[I18N_NOOP("stateMessage")] != printer.stateMsg()) {
        sourceData[I18N_NOOP("stateMessage")] = printer.stateMsg();
        changed = true;
    }
    if (sourceData[I18N_NOOP("iconName")] != printer.iconName()) {
        sourceData[I18N_NOOP("iconName")] = printer.iconName();
        changed = true;
    }

    if (changed) {
        // update only if data changes to avoid uneeded updates on the views
        setData(name, sourceData);
        kDebug() << name << sourceData;
//        forceImmediateUpdateOfAllVisualizations();
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
    setData(name, Data());
    // We do not have any special code to execute the
    // first time a source is requested, so we just call
    // updateSourceEvent().
    return updateSourceEvent(name);
}
 
bool PrintManagerEngine::updateSourceEvent(const QString &name)
{
    kDebug() << name << sender();
 
    KCupsRequest *request = new KCupsRequest;
    request->setProperty("prefix", name);

    if (name == QLatin1String("Printers")) {
            KCupsPrinter::Attributes attr;
            attr |= KCupsPrinter::PrinterName;
            attr |= KCupsPrinter::PrinterInfo;
            attr |= KCupsPrinter::PrinterState;
            attr |= KCupsPrinter::PrinterStateMessage;
            // to get proper icons
            attr |= KCupsPrinter::PrinterType;
            request->getPrinters(attr);
            request->waitTillFinished();
            for (int i = 0; i < request->printers().size(); ++i) {
                printer(i, request->printers().at(i));
            }
    } else {
        QString printer;
        QString whichJob = name;
        QStringList parts = name.split(QLatin1Char('/'));
        if (parts.size() == 3) {
            // Printers PrinterName Kind of the job
            printer = parts.at(1);
            whichJob = parts.at(2);
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
        for (int i = 0; i < request->jobs().size(); ++i) {
            job(i, request->jobs().at(i));
        }
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
