/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti
    dantti12@gmail.com

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef JOB_MODEL_H
#define JOB_MODEL_H

#include <QStandardItemModel>

#include <cups/cups.h>

class KCupsJob;
class KCupsRequest;
class Q_DECL_EXPORT JobModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(JobAction)
    Q_ENUMS(Role)
    Q_ENUMS(WhichJobs)
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
        RoleJobPrinter,
        RoleJobOriginatingHostName,
        RoleJobAuthenticationRequired
    };

    enum JobAction {
        Cancel,
        Hold,
        Release,
        Move,
        Reprint
    };

    enum WhichJobs {
        WhichAll,
        WhichActive,
        WhichCompleted
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
        ColFromHost,
        LastColumn
    };

    explicit JobModel(QObject *parent = nullptr);
    void setParentWId(WId parentId);
    Q_INVOKABLE void init(const QString &destName = QString());

    Q_INVOKABLE void hold(const QString &printerName, int jobId);
    Q_INVOKABLE void release(const QString &printerName, int jobId);
    Q_INVOKABLE void cancel(const QString &printerName, int jobId);
    Q_INVOKABLE void move(const QString &printerName, int jobId, const QString &toPrinterName);

    QString processingJob() const;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDropActions() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data,
                      Qt::DropAction action,
                      int row,
                      int column,
                      const QModelIndex &parent) override;
    virtual QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void setWhichJobs(WhichJobs whichjobs);
    KCupsRequest* modifyJob(int row, JobAction action, const QString &newDestName = QString(), const QModelIndex &parent = QModelIndex());

private slots:
    void getJobs();
    void getJobFinished(KCupsRequest *request);

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
    void clear();

    KCupsRequest *m_jobRequest = nullptr;
    QString m_destName;
    QString m_processingJob;
    QHash<int, QByteArray> m_roles;
    int m_whichjobs = CUPS_WHICHJOBS_ACTIVE;
    WId m_parentId = 0;
};

#endif // JOB_MODEL_H
