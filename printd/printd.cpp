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

#include <KConfigGroup>
#include <KDirWatch>
#include <KGenericFactory>
#include <KIcon>
#include <KMenu>
#include <KStandardDirs>
#include <QHash>
#include <QStringList>

#include <QtCore/QTimer>

#include "PrintQueueTray.h"

#define INTERVAL 2000

K_PLUGIN_FACTORY(PrintDFactory, registerPlugin<PrintD>();)
K_EXPORT_PLUGIN(PrintDFactory("printd"))

PrintD::PrintD(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_trayIcon(0)
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
    int num_jobs;
    cups_job_t *jobs;

    // Get ACTIVE jobs
    num_jobs = cupsGetJobs(&jobs, NULL, m_onlyMyJobs, CUPS_WHICHJOBS_ACTIVE);

    if (num_jobs > 0) {
        if (!m_trayIcon) {
            // Create a new Tray icon that will fill the menu just
            // when it's about to show
            m_trayIcon = new PrintQueueTray(this);
            connect(m_trayIcon->contextMenu(), SIGNAL(aboutToShow()),
                    this, SLOT(fillMenu()));
        }

        m_jobsPerPrinter.clear();

        // Get the number of jobs each printer has
        for (int i = 0; i < num_jobs; i++) {
            m_jobsPerPrinter[QString::fromUtf8(jobs[i].dest)]++;
        }

        QString title, output;
        // If there are more than 3 printers just tell how many jobs are queued
        if (m_jobsPerPrinter.size() > 3) {
            title.append(i18np("One job queued", "%1 jobs queued", num_jobs));
        } else{
            // Get the destination messages
            title = i18n("Printing...");
            QHash<QString, QHash<QString, QString> > messages = destsMessages();
            QHash<QString, int>::const_iterator i = m_jobsPerPrinter.constBegin();
            while (i != m_jobsPerPrinter.constEnd()) {
                if (!output.isEmpty()) {
                    output.append("<br><br>");
                }
                QString destDescription = messages[i.key()]["print-message"];
                if (destDescription.isEmpty()) {
                    destDescription = i.key();
                }
                output.append(i18np("<b>One job in '%2'</b>", "<b>%1 jobs in '%2'</b>", i.value(),
                                    destDescription));
                QString destMessage = messages[i.key()]["printer-state-message"];
                if (!destMessage.isEmpty()) {
                    output.append(QString("<br>'%1'").arg(destMessage));
                }

                ++i;
            }
        }

        if (m_lastSubTitle != output || m_lastTitle != title) {
            m_trayIcon->setToolTip("printer", title, output);
            m_lastSubTitle = output;
            m_lastTitle = title;
        }

        if (m_jobsPerPrinter.size() == 1) {
            // First clear any previously-set associated widget
            m_trayIcon->setAssociatedWidget(0);
            m_trayIcon->connectToLauncher(m_jobsPerPrinter.keys().first());
        } else {
            m_trayIcon->setAssociatedWidget(m_trayIcon->contextMenu());
        }

    } else if (m_trayIcon) {
        // Hide the tray icon if there are no more jobs
        m_trayIcon->deleteLater();
        m_lastSubTitle.clear();
        m_lastTitle.clear();
        m_trayIcon = 0;
    }

    // Free the job array
    cupsFreeJobs(num_jobs, jobs);
}

QHash<QString, QHash<QString, QString> > PrintD::destsMessages() const
{
    ipp_t *request, *response;
    ipp_attribute_t *attr;
    QHash<QString, QHash<QString, QString> > ret;
    static const char * const attrs[] =
                {
                  "printer-name",
                  "printer-state-message"
                };

    request = ippNewRequest(CUPS_GET_PRINTERS);
    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "requested-attributes",
                  (int)(sizeof(attrs) / sizeof(attrs[0])), NULL, attrs);
    if ((response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/")) != NULL) {
        for (attr = response->attrs; attr != NULL; attr = attr->next) {
            /*
             * Skip leading attributes until we hit a printer...
             */
            while (attr != NULL && attr->group_tag != IPP_TAG_PRINTER) {
                attr = attr->next;
            }

            if (attr == NULL) {
                break;
            }

            QString destName;
            QHash<QString, QString> attributes;
            for (; attr && attr->group_tag == IPP_TAG_PRINTER; attr = attr->next) {
                if (attr->value_tag != IPP_TAG_TEXT &&
                    attr->value_tag != IPP_TAG_NAME) {
                    continue;
                }

                if (!strcmp(attr->name, "printer-name") ||
                    !strcmp(attr->name, "printer-state-message")) {
                    attributes[QString::fromUtf8(attr->name)] = QString::fromUtf8(attr->values[0].string.text);
                }
            }

            destName = attributes["printer-name"];
            if (destName.isEmpty()) {
                if (attr == NULL) {
                    break;
                } else {
                  continue;
                }
            }

            ret[destName] = attributes;

            if (attr == NULL) {
                break;
            }
        }

        ippDelete(response);
    }
    return ret;
}

void PrintD::fillMenu()
{
    KMenu *contextMenu = qobject_cast<KMenu*>(sender());
    contextMenu->clear();
    QStringList dests = m_jobsPerPrinter.keys();
    dests.sort();
    contextMenu->addTitle(KIcon("printer"), i18np("Printer", "Printers", dests.size()));
    foreach (const QString &destName, dests) {
        if (!contextMenu->isEmpty()) {
            contextMenu->addSeparator();
        }
        contextMenu->addAction(destName)->setEnabled(false);
        QAction *action = contextMenu->addAction(i18np("One job queued",
                                                       "%1 jobs queued",
                                                       m_jobsPerPrinter[destName]));
        action->setData(destName);
    }
}
