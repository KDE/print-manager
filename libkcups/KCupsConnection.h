/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
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

#ifndef KCUPSCONNECTION_H
#define KCUPSCONNECTION_H

#include <QThread>
#include <QTimer>
#include <QVariantHash>
#include <QStringList>

#include <KUrl>

#include <kdemacros.h>

#include <cups/cups.h>

#define KCUPS_DEVICE_CLASS          "device-class"
#define KCUPS_DEVICE_ID             "device-id"
#define KCUPS_DEVICE_INFO           "device-info"
#define KCUPS_DEVICE_MAKE_AND_MODEL "device-make-and-model"
#define KCUPS_DEVICE_LOCATION       "device-location"
#define KCUPS_DEVICE_URI            "device-uri"

#define KCUPS_PRINTER_NAME                   "printer-name"
#define KCUPS_PRINTER_LOCATION               "printer-location"
#define KCUPS_PRINTER_INFO                   "printer-info"
#define KCUPS_PRINTER_URI                    "printer-uri"
#define KCUPS_PRINTER_MAKE_AND_MODEL         "printer-make-and-model"
#define KCUPS_PRINTER_STATE                  "printer-state"
#define KCUPS_PRINTER_STATE_MESSAGE          "printer-state-message"
#define KCUPS_PRINTER_IS_SHARED              "printer-is-shared"
#define KCUPS_PRINTER_IS_ACCEPTING_JOBS      "printer-is-accepting-jobs"
#define KCUPS_PRINTER_TYPE                   "printer-type"
#define KCUPS_PRINTER_TYPE_MASK              "printer-type-mask"
#define KCUPS_PRINTER_COMMANDS               "printer-commands"
#define KCUPS_PRINTER_URI_SUPPORTED          "printer-uri-supported"
#define KCUPS_PRINTER_ERROR_POLICY           "printer-error-policy"
#define KCUPS_PRINTER_ERROR_POLICY_SUPPORTED "printer-error-policy-supported"
#define KCUPS_PRINTER_OP_POLICY              "printer-op-policy"
#define KCUPS_PRINTER_OP_POLICY_SUPPORTED    "printer-op-policy-supported"

#define KCUPS_MEMBER_URIS  "member-uris"
#define KCUPS_MEMBER_NAMES "member-names"

#define KCUPS_MARKER_CHANGE_TIME "marker-change-time"
#define KCUPS_MARKER_COLORS      "marker-colors"
#define KCUPS_MARKER_LEVELS      "marker-levels"
#define KCUPS_MARKER_HIGH_LEVELS "marker-high-levels"
#define KCUPS_MARKER_LOW_LEVELS  "marker-low-levels"
#define KCUPS_MARKER_NAMES       "marker-names"
#define KCUPS_MARKER_TYPES       "marker-types"
#define KCUPS_MARKER_MESSAGE     "marker-message"

#define KCUPS_JOB_ID                     "job-id"
#define KCUPS_JOB_NAME                   "job-name"
#define KCUPS_JOB_K_OCTETS               "job-k-octets"
#define KCUPS_JOB_K_OCTETS_PROCESSED     "job-k-octets-processed"
#define KCUPS_JOB_PRINTER_URI            "job-printer-uri"
#define KCUPS_JOB_PRINTER_STATE_MESSAGE  "job-printer-state-message"
#define KCUPS_JOB_ORIGINATING_USER_NAME  "job-originating-user-name"
#define KCUPS_JOB_MEDIA_PROGRESS         "job-media-progress"
#define KCUPS_JOB_MEDIA_SHEETS           "job-media-sheets"
#define KCUPS_JOB_MEDIA_SHEETS_COMPLETED "job-media-sheets-completed"
#define KCUPS_JOB_PRESERVED              "job-preserved"
#define KCUPS_JOB_STATE                  "job-state"
#define KCUPS_JOB_SHEETS_DEFAULT         "job-sheets-default"
#define KCUPS_JOB_SHEETS_SUPPORTED       "job-sheets-supported"
#define KCUPS_JOB_SHEETS_DEFAULT         "job-sheets-default"
#define KCUPS_JOB_SHEETS_SUPPORTED       "job-sheets-supported"

#define KCUPS_TIME_AT_COMPLETED  "time-at-completed"
#define KCUPS_TIME_AT_CREATION   "time-at-creation"
#define KCUPS_TIME_AT_PROCESSING "time-at-processing"

#define KCUPS_REQUESTING_USER_NAME_ALLOWED "requesting-user-name-allowed"
#define KCUPS_REQUESTING_USER_NAME_DENIED  "requesting-user-name-denied"

typedef QList<QVariantHash> ReturnArguments;
class KCupsPasswordDialog;
class KDE_EXPORT KCupsConnection : public QThread
{
    Q_OBJECT
public:
    /**
     * This is the main Cups class @author Daniel Nicoletti <dantti12@gmail.com>
     *
     * By calling KCupsConnection::global() you have access to it.
     * Due to cups archtecture, this class has to live on a
     * separate thread so we avoid blocking the user interface when
     * the cups call blocks.
     *
     * It is IMPORTANT that we do not create several thread
     * for each cups request, doing so is a valid but breaks our
     * authentication. We could tho store the user information an
     * set the user/password every time it was needed. But I am not
     * sure this is safe.
     *
     * Extending this means either adding methods to the KCupsRequest
     * class which will move to this thread and then run.
     */
    static KCupsConnection* global();

    /**
     * @brief KCupsConnection
     * @param parent
     *
     * This is the default constructor that connects to the default server
     * If you don't have any special reason for creating a connection
     * on your own consider calling global()
     */
    explicit KCupsConnection(QObject *parent = 0);
    KCupsConnection(const KUrl &server, QObject *parent = 0);
    ~KCupsConnection();

Q_SIGNALS:
    /**
     * emitted when "server-started" is registered
     */
    void serverStarted(const QString &text);

    /**
     * emitted when "server-stopped" is registered
     */
    void serverStopped(const QString &text);

    /**
     * emitted when "server-restarted" is registered
     */
    void serverRestarted(const QString &text);

    /**
     * emitted when "server-audit" is registered
     */
    void serverAudit(const QString &text);


    /**
     * emitted when "printer-added" is registered
     */
    void printerAdded(const QString &text,
                      const QString &printerUri,
                      const QString &printerName,
                      uint printerState,
                      const QString &printerStateReasons,
                      bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-modified" is registered
     */
    void printerModified(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-deleted" is registered
     */
    void printerDeleted(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-state-changed" is registered
     */
    void printerStateChanged(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-stopped" is registered
     */
    void printerStopped(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-restarted" is registered
     */
    void printerRestarted(const QString &text,
                          const QString &printerUri,
                          const QString &printerName,
                          uint printerState,
                          const QString &printerStateReasons,
                          bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-shutdown" is registered
     */
    void printerShutdown(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-media-changed" is registered
     */
    void printerMediaChanged(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);

    /**
     * emitted when "printer-finishings-changed" is registered
     */
    void printerFinishingsChanged(const QString &text,
                                  const QString &printerUri,
                                  const QString &printerName,
                                  uint printerState,
                                  const QString &printerStateReasons,
                                  bool printerIsAcceptingJobs);


    /**
     * emitted when "job-state-changed" is registered
     */
    void jobState(const QString &text,
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

    /**
     * emitted when "job-created" is registered
     */
    void jobCreated(const QString &text,
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

    /**
     * emitted when "job-stopped" is registered
     */
    void jobStopped(const QString &text,
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

    /**
     * emitted when "job-config-changed" is registered
     */
    void jobConfigChanged(const QString &text,
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

    /**
     * emitted when "job-progress" is registered
     */
    void jobProgress(const QString &text,
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

    /**
     * emitted when "job-completed" is registered
     */
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

    void rhPrinterAdded(const QString &queueName);
    void rhPrinterRemoved(const QString &queueName);
    void rhQueueChanged(const QString &queueName);
    void rhJobQueuedLocal(const QString &queueName, uint jobId, const QString &jobOwner);
    void rhJobStartedLocal(const QString &queueName, uint jobId, const QString &jobOwner);

protected:
    friend class KCupsRequest;

    virtual void run();
    bool readyToStart();
    bool retry(const char *resource);
    ReturnArguments request(ipp_op_e operation,
                            const char *resource,
                            const QVariantHash &reqValues,
                            bool needResponse);

private slots:
    void updateSubscription();
    void renewDBusSubscription();
    void cancelDBusSubscription();

protected:
    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);
    QString eventForSignal(const char *signal) const;

private:
    void init();
    /**
     * This is the most weird cups function, the DBus API
     * it is completely messy, and if we change the order of the attributes
     * the call just fails. This is why we have a specific method
     */
    int renewDBusSubscription(int subscriptionId, int leaseDuration, const QStringList &events = QStringList());

    void notifierConnect(const QString &signal, QObject *receiver, const char *slot);

    static void requestAddValues(ipp_t *request, const QVariantHash &values);
    static ReturnArguments parseIPPVars(ipp_t *response,
                                        int group_tag,
                                        bool needDestName);
    static ipp_t* ippNewDefaultRequest(const QString &name, bool isClass, ipp_op_t operation);
    static QVariant ippAttrToVariant(ipp_attribute_t *attr);

    static KCupsConnection* m_instance;

    bool m_inited;
    KCupsPasswordDialog *m_passwordDialog;
    KUrl m_serverUrl;

    QTimer *m_subscriptionTimer;
    QTimer *m_renewTimer;
    QStringList m_connectedEvents;
    QStringList m_requestedDBusEvents;
    int m_subscriptionId;
};

#endif // KCUPSCONNECTION_H
