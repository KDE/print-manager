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

#include <kdemacros.h>

class KCupsJob;
class KCupsRequest;
class KDE_EXPORT PrintQueueModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(JobAction)
    Q_ENUMS(Role)
public:
    enum Role {
        RoleJobId = Qt::UserRole + 2,
        RoleJobState,
        RoleJobName,
        RoleJobPages,
        RoleJobSize,
        RoleJobOwner,
        RoleJobCreatedAt,
        RoleJobIconName,
        RoleJobCancelEnabled,
        RoleJobHoldEnabled,
        RoleJobReleaseEnabled,
        RoleJobRestartEnabled,
        RoleJobPrinter
    };

    enum JobAction {
        Cancel,
        Hold,
        Release,
        Move
    };

    enum Columns {
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
    };

    explicit PrintQueueModel(QObject *parent = 0);
    void setParentWId(WId parentId);
    Q_INVOKABLE void init(const QString &destName = QString());
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

    void createSubscription();
    void createSubscriptionFinished();
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

    KCupsRequest *m_jobRequest;
    QString m_destName;
    QString m_processingJob;
    int m_whichjobs;
    WId m_parentId;
    QStringList m_jobAttributes;
    int m_subscriptionId;
};

#endif
