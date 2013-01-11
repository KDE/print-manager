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

#ifndef PRINT_JOBS_ENGINE_H
#define PRINT_JOBS_ENGINE_H
 
#include <Plasma/DataEngine>

#include <KCupsRequest.h>
 
/**
 * This engine provides all printers the current server has.
 *
 */
class KCupsPrinter;
class KCupsRequest;
class PrintJobsEngine : public Plasma::DataEngine
{
    Q_OBJECT
public:
    // every engine needs a constructor with these arguments
    PrintJobsEngine(QObject *parent, const QVariantList &args);
    ~PrintJobsEngine();

    // Get and set all the jobs we have
    virtual void init();

    // Get the Service class which we run operations on
    virtual Plasma::Service* serviceForSource(const QString &source);

private slots:
    void getJobs();
    void getJobsFinished();
    void jobCompleted(const QString &text,
                      const QString &printerUri,
                      const QString &printerName,
                      uint printerState,
                      const QString &printerStateReasons,
                      bool printerIsAcceptingJobs,
                      uint jobId,
                      uint jobState,
                      const QString &jobStateReasons,
                      const QString &jobName,
                      uint jobImpressionsCompleted);
    void insertUpdateJob(uint jobId, const QString &printerUri);
    void insertUpdateJob(const QString &queueName, uint jobId, const QString &jobOwner);
    void insertUpdateJob(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs,
                         uint jobId,
                         uint jobState,
                         const QString &jobStateReasons,
                         const QString &jobName,
                         uint jobImpressionsCompleted);
    void insertUpdateJobFinished();

private:
    void updateJobSource(const KCupsJob &job);
    bool updateJobState(Plasma::DataEngine::Data &sourceData, ipp_jstate_t jobState);

    QStringList m_jobAttributes;
    KCupsRequest *m_jobRequest;
};
 
#endif // PRINT_JOBS_ENGINE_H
