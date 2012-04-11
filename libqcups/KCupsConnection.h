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

#include <KPasswordDialog>

#include <cups/cups.h>

typedef QList<QVariantHash> ReturnArguments;
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

protected:
    friend class KCupsRequest;

    virtual void run();
    static bool readyToStart();
    static bool retryIfForbidden();
    /**
      * Always use this method to get the last error
      * because if the cups connection fails, the last
      * error will be an internal error and we need
      * to destroy this thread in order to recover
      * from this error.
      */
    static ipp_status_t lastError();

    static ReturnArguments request(ipp_op_e operation,
                                   const QString &resource,
                                   const QVariantHash &reqValues,
                                   bool needResponse);

    /**
     * Just pass the list of event and this class will
     * worry about renewing
     */
    int createDBusSubscription(const QStringList &events);

    /**
     * This is the most weird cups function, the DBus API
     * is completely messy, and if we change the order of the attributes
     * the call just fails. This is why we have a specific method
     */
    void removeDBusSubscription(int subscriptionId);

private slots:
    void renewDBusSubscription();
    void cancelDBusSubscription();

private:
    KCupsConnection(QObject *parent = 0);
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
    KPasswordDialog *m_passwordDialog;

    QTimer *m_renewTimer;
    QMap<int, QStringList> m_requestedDBusEvents;
    int m_subscriptionId;
};

#endif // KCUPSCONNECTION_H
