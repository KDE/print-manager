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

#ifndef PRINT_MANAGER_ENGINE_H
#define PRINT_MANAGER_ENGINE_H
 
#include <Plasma/DataEngine>

#include <KCupsJob.h>
 
/**
 * This engine provides all the print jobs the current server has.
 *
 * "AllJobs" lists all jobs of all printers
 * "ActiveJobs" lists active jobs of all printers
 * "CompletedJobs" lists completed jobs of all printers
 * "Printers" lists all printers
 * "Printers/printer_name/AllJobs" lists all jobs from the printer "printer_name"
 * "Printers/printer_name/ActiveJobs" lists active jobs from the printer "printer_name"
 * "Printers/printer_name/CompletedJobs" lists completed jobs from the printer "printer_name"
 */
class KCupsPrinter;
class PrintManagerEngine : public Plasma::DataEngine
{
    Q_OBJECT
public:
    // every engine needs a constructor with these arguments
    PrintManagerEngine(QObject *parent, const QVariantList &args);

    // Get and set all the jobs we have
    virtual void init();

    // Get the Service class which we run operations on
    virtual Plasma::Service* serviceForSource(const QString &source);

private slots:
    void job(int order, const KCupsJob &job);
    void printer(int order, const KCupsPrinter &printer);

protected:
    // this virtual function is called when a new source is requested
    bool sourceRequestEvent(const QString &name);

    // this virtual function is called when an automatic update
    // is triggered for an existing source (ie: when a valid update
    // interval is set when requesting a source)
    bool updateSourceEvent(const QString &source);

private:
    KCupsJob::Attributes m_jobAttributes;
};
 
#endif // PRINT_MANAGER_ENGINE_H
