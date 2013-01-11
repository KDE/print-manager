/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#ifndef PRINT_QUEUE_MODEL_H
#define PRINT_QUEUE_MODEL_H

#include <QStandardItemModel>

#include <cups/cups.h>

class KCupsJob;
class KCupsPrinter;
class KCupsRequest;
class PrintQueueModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(JobAction)
    Q_ENUMS(Role)
public:
    typedef enum {
        JobId = Qt::UserRole + 2,
        JobState,
        DestName
    } Role;

    typedef enum {
        Cancel,
        Hold,
        Release,
        Move
    } JobAction;

    typedef enum {
        ColStatus = 0,
        ColName,
        ColUser,
        ColCreated,
        ColCompleted,
        ColPages,
        ColProcessed,
        ColSize,
        ColStatusMessage,
        ColPrinter,
        LastColumn
    } Columns;

    explicit PrintQueueModel(const QString &destName, WId parentId, QObject *parent = 0);
    void init();
    QString processingJob() const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data,
                      Qt::DropAction action,
                      int row,
                      int column,
                      const QModelIndex &parent);

    void setWhichJobs(int whichjobs);
    KCupsRequest* modifyJob(int row, JobAction action, const QString &newDestName = QString(), const QModelIndex &parent = QModelIndex());

private slots:
    void getJobs();
    void getJobFinished();

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

private:
    int jobRow(int jobId);
    void insertJob(int pos, const KCupsJob &job);
    void updateJob(int pos, const KCupsJob &job);
    QString jobStatus(ipp_jstate_e job_state);

    KCupsPrinter *m_printer;
    KCupsRequest *m_jobRequest;
    QString m_destName;
    QString m_processingJob;
    int m_whichjobs;
    WId m_parentId;
    QStringList m_jobAttributes;
};

#endif
