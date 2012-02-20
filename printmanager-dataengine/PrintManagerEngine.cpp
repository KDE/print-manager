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
    setMinimumPollingInterval(333);
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
        kDebug() << id << job.id();
    setData(id, I18N_NOOP("order"), order);
    setData(id, I18N_NOOP("jobName"), job.name());
    setData(id, I18N_NOOP("jobKOctets"), job.size());
//    setData(id, I18N_NOOP("jobKOctetsProcessed"), job.name());
    setData(id, I18N_NOOP("jobState"), job.state());
    setData(id, I18N_NOOP("timeAtCompleted"), job.completedAt());
    setData(id, I18N_NOOP("timeAtCreation"), job.createdAt());
    setData(id, I18N_NOOP("timeAtProcessing"), job.processedAt());
    setData(id, I18N_NOOP("jobPrinterUri"), job.printer());
    setData(id, I18N_NOOP("jobOriginatingUserName"), job.ownerName());
//    setData(id, I18N_NOOP("jobMediaProgress"), job.());
//    setData(id, I18N_NOOP("jobMediaSheets"), job.name());
    setData(id, I18N_NOOP("jobMediaSheetsCompleted"), job.completedPages());
//    setData(id, I18N_NOOP("jobPrinterStatMessage"), job.name());
//    setData(id, I18N_NOOP("jobPreserved"), job.name());
}

void PrintManagerEngine::printer(int order, const KCupsPrinter &printer)
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    QString name = printer.name();
    name.prepend(request->property("prefix").toString() + QLatin1Char('/'));
    kDebug() << order << name;
    setData(name, I18N_NOOP("order"), order);
    setData(name, I18N_NOOP("printerName"), printer.name());
    setData(name, I18N_NOOP("info"), printer.info());
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
    setData(name, I18N_NOOP("stateEnum"), state);
    setData(name, I18N_NOOP("stateMessage"), printer.stateMsg());
    setData(name, I18N_NOOP("iconName"), printer.iconName());
}
 
bool PrintManagerEngine::sourceRequestEvent(const QString &name)
{
    kDebug() << name;
    // We do not have any special code to execute the
    // first time a source is requested, so we just call
    // updateSourceEvent().
    return updateSourceEvent(name);
}
 
bool PrintManagerEngine::updateSourceEvent(const QString &name)
{
    kDebug() << name;
 
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(job(int,KCupsJob)), this, SLOT(job(int,KCupsJob)));
    connect(request, SIGNAL(printer(int,KCupsPrinter)), this, SLOT(printer(int,KCupsPrinter)));
    request->setProperty("prefix", name);

    if (name == I18N_NOOP("Printers")) {
            KCupsPrinter::Attributes attr;
            attr |= KCupsPrinter::PrinterName;
            attr |= KCupsPrinter::PrinterInfo;
            attr |= KCupsPrinter::PrinterState;
            attr |= KCupsPrinter::PrinterStateMessage;
            // to get proper icons
            attr |= KCupsPrinter::PrinterType;
            request->getPrinters(attr);
    } else {
        QString printer;
        QString whichJob = name;
        QStringList parts = name.split(QLatin1Char('/'));
        if (parts.size() == 3) {
            // Printers PrinterName Kind of the job
            printer = parts.at(1);
            whichJob = parts.at(2);
        }

        if (whichJob == I18N_NOOP("AllJobs")) {
            request->getJobs(printer, false, CUPS_WHICHJOBS_ALL, m_jobAttributes);
        } else if (whichJob == I18N_NOOP("ActiveJobs")) {
            request->getJobs(printer, false, CUPS_WHICHJOBS_ACTIVE, m_jobAttributes);
        } else if (whichJob == I18N_NOOP("CompletedJobs")) {
            request->getJobs(printer, false, CUPS_WHICHJOBS_COMPLETED, m_jobAttributes);
        }
    }

    request->waitTillFinished();
    request->deleteLater();
    return !request->hasError();
}
 
// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// The second argument is the name of the class in
// your plugin that derives from Plasma::DataEngine
K_EXPORT_PLASMA_DATAENGINE(testtime, PrintManagerEngine)
 
// this is needed since PrintManagerEngine is a QObject
#include "PrintManagerEngine.moc"
