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

#include "printd.h"

#include <KGenericFactory>
#include <KStandardDirs>
#include <KConfigGroup>
#include <KDirWatch>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

#include <cups/cups.h>

#define INTERVAL 5000

#define SYSTRAY_NAME "org.kde.PrintQueue"

K_PLUGIN_FACTORY(PrintDFactory, registerPlugin<PrintD>();)
K_EXPORT_PLUGIN(PrintDFactory("printd"))

PrintD::PrintD(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
{
    // Read kded config
    readConfig();

    // Check if any changes to the conifg file occours
    KDirWatch *configWatch = new KDirWatch(this);
    configWatch->addFile(KStandardDirs::locateLocal("config", "print-manager"));
    connect(configWatch, SIGNAL(  dirty(const QString &)), this, SLOT(readConfig()));
    connect(configWatch, SIGNAL(created(const QString &)), this, SLOT(readConfig()));
    connect(configWatch, SIGNAL(deleted(const QString &)), this, SLOT(readConfig()));
    configWatch->startScan();

    m_jobsTimer = new QTimer(this);
    m_jobsTimer->setInterval(INTERVAL);
    connect(m_jobsTimer, SIGNAL(timeout()), this, SLOT(checkJobs()));
    m_jobsTimer->start();

    connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString&)),
            this, SLOT(serviceOwnerChanged(const QString&, const QString&, const QString&)));
}

PrintD::~PrintD()
{
}

void PrintD::readConfig()
{
    KConfig config("print-manager");
    KConfigGroup monitor(&config, "Monitor");
    // If we should watch all jobs
    m_onlyMyJobs = monitor.readEntry("OnlyMyJobs", true);
}

void PrintD::checkJobs()
{
    //     m_jobsTimer->stop();
    int num_jobs;
    cups_job_t *jobs;

    // Get ACTIVE jobs
    num_jobs = cupsGetJobs(&jobs, NULL, m_onlyMyJobs, CUPS_WHICHJOBS_ACTIVE);

    if (num_jobs > 0) {
        // Start the printer icon
        m_trayIcon = new PrintQueueTray;
    }

    // Free the job array
    cupsFreeJobs(num_jobs, jobs);
}

void PrintD::serviceOwnerChanged(const QString &name, const QString &oldOnwer, const QString &newOwner)
{
    Q_UNUSED(oldOnwer)
    if (name != SYSTRAY_NAME) {
        return;
    }

    // newOwner is empty when the process exited, else it started
    if (newOwner.isEmpty()) {
        m_jobsTimer->start();
    } else {
        m_jobsTimer->stop();
    }
}
