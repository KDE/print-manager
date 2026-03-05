/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCUPSCONNECTION_H
#define KCUPSCONNECTION_H

#include <QMetaMethod>
#include <QMutex>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>
#include <QWidget>

#include <kcups_export.h>

#include <cups/cups.h>

constexpr QLatin1String KCUPS_DEVICE_CLASS("device-class");
constexpr QLatin1String KCUPS_DEVICE_ID("device-id");
constexpr QLatin1String KCUPS_DEVICE_INFO("device-info");
constexpr QLatin1String KCUPS_DEVICE_MAKE_AND_MODEL("device-make-and-model");
constexpr QLatin1String KCUPS_DEVICE_LOCATION("device-location");
constexpr QLatin1String KCUPS_DEVICE_URI("device-uri");

constexpr QLatin1String KCUPS_PRINTER_NAME("printer-name");
constexpr QLatin1String KCUPS_PRINTER_LOCATION("printer-location");
constexpr QLatin1String KCUPS_PRINTER_INFO("printer-info");
constexpr QLatin1String KCUPS_PRINTER_URI("printer-uri");
constexpr QLatin1String KCUPS_PRINTER_MAKE_AND_MODEL("printer-make-and-model");
constexpr QLatin1String KCUPS_PRINTER_STATE("printer-state");
constexpr QLatin1String KCUPS_PRINTER_STATE_MESSAGE("printer-state-message");
constexpr QLatin1String KCUPS_PRINTER_IS_SHARED("printer-is-shared");
constexpr QLatin1String KCUPS_PRINTER_IS_ACCEPTING_JOBS("printer-is-accepting-jobs");
constexpr QLatin1String KCUPS_PRINTER_TYPE("printer-type");
constexpr QLatin1String KCUPS_PRINTER_TYPE_MASK("printer-type-mask");
constexpr QLatin1String KCUPS_PRINTER_COMMANDS("printer-commands");
constexpr QLatin1String KCUPS_PRINTER_URI_SUPPORTED("printer-uri-supported");
constexpr QLatin1String KCUPS_PRINTER_ERROR_POLICY("printer-error-policy");
constexpr QLatin1String KCUPS_PRINTER_ERROR_POLICY_SUPPORTED("printer-error-policy-supported");
constexpr QLatin1String KCUPS_PRINTER_OP_POLICY("printer-op-policy");
constexpr QLatin1String KCUPS_PRINTER_OP_POLICY_SUPPORTED("printer-op-policy-supported");

constexpr QLatin1String KCUPS_MEMBER_URIS("member-uris");
constexpr QLatin1String KCUPS_MEMBER_NAMES("member-names");

constexpr QLatin1String KCUPS_MARKER_CHANGE_TIME("marker-change-time");
constexpr QLatin1String KCUPS_MARKER_COLORS("marker-colors");
constexpr QLatin1String KCUPS_MARKER_LEVELS("marker-levels");
constexpr QLatin1String KCUPS_MARKER_HIGH_LEVELS("marker-high-levels");
constexpr QLatin1String KCUPS_MARKER_LOW_LEVELS("marker-low-levels");
constexpr QLatin1String KCUPS_MARKER_NAMES("marker-names");
constexpr QLatin1String KCUPS_MARKER_TYPES("marker-types");
constexpr QLatin1String KCUPS_MARKER_MESSAGE("marker-message");

constexpr QLatin1String KCUPS_JOB_ID("job-id");
constexpr QLatin1String KCUPS_JOB_NAME("job-name");
constexpr QLatin1String KCUPS_JOB_K_OCTETS("job-k-octets");
constexpr QLatin1String KCUPS_JOB_K_OCTETS_PROCESSED("job-k-octets-processed");
constexpr QLatin1String KCUPS_JOB_PRINTER_URI("job-printer-uri");
constexpr QLatin1String KCUPS_JOB_PRINTER_STATE_MESSAGE("job-printer-state-message");
constexpr QLatin1String KCUPS_JOB_ORIGINATING_USER_NAME("job-originating-user-name");
constexpr QLatin1String KCUPS_JOB_ORIGINATING_HOST_NAME("job-originating-host-name");
constexpr QLatin1String KCUPS_JOB_MEDIA_PROGRESS("job-media-progress");
constexpr QLatin1String KCUPS_JOB_MEDIA_SHEETS("job-media-sheets");
constexpr QLatin1String KCUPS_JOB_MEDIA_SHEETS_COMPLETED("job-media-sheets-completed");
constexpr QLatin1String KCUPS_JOB_PRESERVED("job-preserved");
constexpr QLatin1String KCUPS_JOB_STATE("job-state");
constexpr QLatin1String KCUPS_JOB_STATE_REASONS("job-state-reasons");
constexpr QLatin1String KCUPS_JOB_HOLD_UNTIL("job-hold-until");
constexpr QLatin1String KCUPS_JOB_SHEETS_DEFAULT("job-sheets-default");
constexpr QLatin1String KCUPS_JOB_SHEETS_SUPPORTED("job-sheets-supported");

constexpr QLatin1String KCUPS_MY_JOBS("my-jobs");
constexpr QLatin1String KCUPS_WHICH_JOBS("which-jobs");

constexpr QLatin1String KCUPS_TIME_AT_COMPLETED("time-at-completed");
constexpr QLatin1String KCUPS_TIME_AT_CREATION("time-at-creation");
constexpr QLatin1String KCUPS_TIME_AT_PROCESSING("time-at-processing");

constexpr QLatin1String KCUPS_REQUESTED_ATTRIBUTES("requested-attributes");

constexpr QLatin1String KCUPS_REQUESTING_USER_NAME("requesting-user-name");
constexpr QLatin1String KCUPS_REQUESTING_USER_NAME_ALLOWED("requesting-user-name-allowed");
constexpr QLatin1String KCUPS_REQUESTING_USER_NAME_DENIED("requesting-user-name-denied");

constexpr QLatin1String KCUPS_PPD_MAKE_AND_MODEL("ppd-make-and-model");

constexpr QLatin1String KCUPS_NOTIFY_EVENTS("notify-events");
constexpr QLatin1String KCUPS_NOTIFY_PULL_METHOD("notify-pull-method");
constexpr QLatin1String KCUPS_NOTIFY_RECIPIENT_URI("notify-recipient-uri");
constexpr QLatin1String KCUPS_NOTIFY_LEASE_DURATION("notify-lease-duration");
constexpr QLatin1String KCUPS_NOTIFY_SUBSCRIPTION_ID("notify-subscription-id");

constexpr QLatin1String KCUPS_AUTH_INFO("auth-info");
constexpr QLatin1String KCUPS_AUTH_INFO_REQUIRED("auth-info-required");

typedef QList<QVariantMap> ReturnArguments;

class KIppRequest;
class KCupsPasswordDialog;
class KCUPS_EXPORT KCupsConnection : public QThread
{
    Q_OBJECT
public:
    /**
     * This is the main Cups class @author Daniel Nicoletti <dantti12@gmail.com>
     *
     * By calling KCupsConnection::global() you have access to it.
     * Due to cups architecture, this class has to live on a
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
    static KCupsConnection *global();

    /**
     * @brief KCupsConnection
     * @param parent
     *
     * This is the default constructor that connects to the default server
     * If you don't have any special reason for creating a connection
     * on your own consider calling global()
     */
    explicit KCupsConnection(QObject *parent = nullptr);
    explicit KCupsConnection(const QUrl &server, QObject *parent = nullptr);
    ~KCupsConnection() override;

    void setPasswordMainWindow(WId mainwindow);

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

protected:
    friend class KCupsRequest;

    void run() override;
    bool readyToStart();
    bool retry(const char *resource, int operation) const;
    ReturnArguments request(http_t *http, const KIppRequest &request, ipp_tag_t groupTag = IPP_TAG_ZERO) const;

private Q_SLOTS:
    void updateSubscription();
    void renewDBusSubscription();
    void cancelDBusSubscription();

protected:
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    QString eventForSignal(const QMetaMethod &signal) const;

private:
    void init();

    int renewDBusSubscription(int subscriptionId, int leaseDuration, const QStringList &events = QStringList());

    void notifierConnect(const QString &signal, QObject *receiver, const char *slot);

    static ReturnArguments parseIPPVars(ipp_t *response, ipp_tag_t group_tag);
    static QVariant ippAttrToVariant(ipp_attribute_t *attr);

    static KCupsConnection *m_instance;

    KCupsPasswordDialog *m_passwordDialog;
    QUrl m_serverUrl;

    QTimer *m_subscriptionTimer;
    QTimer *m_renewTimer;
    QStringList m_connectedEvents; // note this updated in another thread. Always guard with m_mutex
    QStringList m_requestedDBusEvents;
    int m_subscriptionId = -1;
    QMutex m_mutex;
};

#endif // KCUPSCONNECTION_H
