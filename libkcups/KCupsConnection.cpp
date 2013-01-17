/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
 *   Copyright (C) 2012 Harald Sitter <sitter@kde.org>                     *
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

#include "KCupsConnection.h"

#include "KCupsPasswordDialog.h"

#include <config.h>

#include <QCoreApplication>
#include <QStringBuilder>
#include <QDBusConnection>
#include <QByteArray>

#include <KLocale>
#include <KDebug>

#include <cups/cups.h>

#define RENEW_INTERVAL        3500
#define SUBSCRIPTION_DURATION 3600

#define DBUS_SERVER_RESTARTED "server-restarted" // ServerRestarted
#define DBUS_SERVER_STARTED   "server-started"   // ServerStarted
#define DBUS_SERVER_STOPPED   "server-stopped"   // ServerStopped
#define DBUS_SERVER_AUDIT     "server-audit"     // ServerAudit

#define DBUS_PRINTER_RESTARTED          "printer-restarted"          // PrinterRestarted
#define DBUS_PRINTER_SHUTDOWN           "printer-shutdown"           // PrinterShutdown
#define DBUS_PRINTER_STOPPED            "printer-stopped"            // PrinterStopped
#define DBUS_PRINTER_STATE_CHANGED      "printer-state-changed"      // PrinterStateChanged
#define DBUS_PRINTER_FINISHINGS_CHANGED "printer-finishings-changed" // PrinterFinishingsChanged
#define DBUS_PRINTER_MEDIA_CHANGED      "printer-media-changed"      // PrinterMediaChanged
#define DBUS_PRINTER_ADDED              "printer-added"              // PrinterAdded
#define DBUS_PRINTER_DELETED            "printer-deleted"            // PrinterDeleted
#define DBUS_PRINTER_MODIFIED           "printer-modified"           // PrinterModified

#define DBUS_JOB_STATE_CHANGED  "job-state-changed"  // JobState
#define DBUS_JOB_CREATED        "job-created"        // JobCreated
#define DBUS_JOB_COMPLETED      "job-completed"      // JobCompleted
#define DBUS_JOB_STOPPED        "job-stopped"        // JobStopped
#define DBUS_JOB_CONFIG_CHANGED "job-config-changed" // JobConfigChanged
#define DBUS_JOB_PROGRESS       "job-progress"       // JobProgress

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<bool>)

KCupsConnection* KCupsConnection::m_instance = 0;
static int password_retries = 0;
static int internalErrorCount = 0;
const char * password_cb(const char *prompt, http_t *http, const char *method, const char *resource, void *user_data);

static const char **qStringListToCharPtrPtr(const QStringList &list, QList<QByteArray> *qbaList)
{
    const char **ptr = new const char *[list.size() + 1];
    qbaList->reserve(qbaList->size() + list.size());
    QByteArray qba;
    for (int i = 0; i < list.size(); ++i) {
        qba = list.at(i).toUtf8();
        qbaList->append(qba);
        ptr[i] = qba.constData();
    }
    ptr[list.size()] = 0;
    return ptr;
}

KCupsConnection* KCupsConnection::global()
{
    if (!m_instance) {
        m_instance = new KCupsConnection(qApp);
    }

    return m_instance;
}

KCupsConnection::KCupsConnection(QObject *parent) :
    QThread(parent)
{
    init();
}

KCupsConnection::KCupsConnection(const KUrl &server, QObject *parent) :
    QThread(parent),
    m_serverUrl(server)
{
    init();
}

KCupsConnection::~KCupsConnection()
{
    if (m_instance == this) {
        m_instance = 0;
    }
    m_passwordDialog->deleteLater();

    quit();
    wait();

    delete m_renewTimer;
    delete m_subscriptionTimer;
}

void KCupsConnection::init()
{
    // Creating the dialog before start() will make it run on the gui thread
    m_passwordDialog = new KCupsPasswordDialog;
    m_subscriptionId = -1;
    m_inited = false;

    // setup the DBus subscriptions

    // Server related signals
    // ServerStarted
    notifierConnect(QLatin1String("ServerStarted"),
                    this,
                    SIGNAL(serverStarted(QString)));

    // ServerStopped
    notifierConnect(QLatin1String("ServerStopped"),
                    this,
                    SIGNAL(serverStopped(QString)));

    // ServerRestarted
    notifierConnect(QLatin1String("ServerRestarted"),
                    this,
                    SIGNAL(serverRestarted(QString)));

    // ServerAudit
    notifierConnect(QLatin1String("ServerAudit"),
                    this,
                    SIGNAL(serverAudit(QString)));

    // Printer related signals
    // PrinterAdded
    notifierConnect(QLatin1String("PrinterAdded"),
                    this,
                    SIGNAL(printerAdded(QString,QString,QString,uint,QString,bool)));

    // PrinterModified
    notifierConnect(QLatin1String("PrinterModified"),
                    this,
                    SIGNAL(printerModified(QString,QString,QString,uint,QString,bool)));

    // PrinterDeleted
    notifierConnect(QLatin1String("PrinterDeleted"),
                    this,
                    SIGNAL(printerDeleted(QString,QString,QString,uint,QString,bool)));

    // PrinterStateChanged
    notifierConnect(QLatin1String("PrinterStateChanged"),
                    this,
                    SIGNAL(printerStateChanged(QString,QString,QString,uint,QString,bool)));

    // PrinterStopped
    notifierConnect(QLatin1String("PrinterStopped"),
                    this,
                    SIGNAL(printerStopped(QString,QString,QString,uint,QString,bool)));

    // PrinterShutdown
    notifierConnect(QLatin1String("PrinterShutdown"),
                    this,
                    SIGNAL(printerShutdown(QString,QString,QString,uint,QString,bool)));

    // PrinterRestarted
    notifierConnect(QLatin1String("PrinterRestarted"),
                    this,
                    SIGNAL(printerRestarted(QString,QString,QString,uint,QString,bool)));

    // PrinterMediaChanged
    notifierConnect(QLatin1String("PrinterMediaChanged"),
                    this,
                    SIGNAL(printerMediaChanged(QString,QString,QString,uint,QString,bool)));

    // PrinterFinishingsChanged
    notifierConnect(QLatin1String("PrinterFinishingsChanged"),
                    this,
                    SIGNAL(PrinterFinishingsChanged(QString,QString,QString,uint,QString,bool)));

    // Job related signals
    // JobState
    notifierConnect(QLatin1String("JobState"),
                    this,
                    SIGNAL(jobState(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // JobCreated
    notifierConnect(QLatin1String("JobCreated"),
                    this,
                    SIGNAL(jobCreated(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // JobStopped
    notifierConnect(QLatin1String("JobStopped"),
                    this,
                    SIGNAL(jobStopped(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // JobConfigChanged
    notifierConnect(QLatin1String("JobConfigChanged"),
                    this,
                    SIGNAL(jobConfigChanged(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // JobProgress
    notifierConnect(QLatin1String("JobProgress"),
                    this,
                    SIGNAL(jobProgress(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // JobCompleted
    notifierConnect(QLatin1String("JobCompleted"),
                    this,
                    SIGNAL(jobCompleted(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint)));

    // This signal is needed since the cups registration thing
    // doesn't emit printerAdded when we add a printer class
    // This is emitted when a printer/queue is changed
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("PrinterAdded"),
                                         this,
                                         SIGNAL(rhPrinterAdded(QString)));

    // This signal is needed since the cups registration thing
    // sometimes simple stops working... don't ask me why
    // This is emitted when a printer/queue is changed
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("QueueChanged"),
                                         this,
                                         SIGNAL(rhQueueChanged(QString)));

    // This signal is needed since the cups registration thing
    // doesn't emit printerRemoved when we add a printer class
    // This is emitted when a printer/queue is changed
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("PrinterRemoved"),
                                         this,
                                         SIGNAL(rhPrinterRemoved(QString)));

    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("JobQueuedLocal"),
                                         this,
                                         SIGNAL(rhJobQueuedLocal(QString,uint,QString)));

    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String("/com/redhat/PrinterSpooler"),
                                         QLatin1String("com.redhat.PrinterSpooler"),
                                         QLatin1String("JobStartedLocal"),
                                         this,
                                         SIGNAL(rhJobStartedLocal(QString,uint,QString)));

    // Creates the timer that will renew the DBus subscription
    m_renewTimer = new QTimer;
    m_renewTimer->setInterval(RENEW_INTERVAL*1000);
    m_renewTimer->moveToThread(this);
    connect(m_renewTimer, SIGNAL(timeout()), this, SLOT(renewDBusSubscription()), Qt::DirectConnection);

    // Creates the timer to merge updates on the DBus subscription
    m_subscriptionTimer = new QTimer;
    m_subscriptionTimer->setInterval(0);
    m_subscriptionTimer->setSingleShot(true);
    m_subscriptionTimer->moveToThread(this);
    connect(m_subscriptionTimer, SIGNAL(timeout()), this, SLOT(updateSubscription()), Qt::DirectConnection);

    // Starts this thread
    start();
}


void KCupsConnection::run()
{
    // Check if we need an special connection
    if (!m_serverUrl.isEmpty()) {
        if (m_serverUrl.port() < 0) {
            // TODO find out if there's a better way of hardcoding
            // the CUPS port
            m_serverUrl.setPort(631);
        }

        cupsSetServer(m_serverUrl.authority().toUtf8());
    }

    // This is dead cool, cups will call the thread_password_cb()
    // function when a password set is needed, as we passed the
    // password dialog pointer the functions just need to call
    // it on a blocking mode.
    cupsSetPasswordCB2(password_cb, m_passwordDialog);

    m_inited = true;
    exec();

    // Event loop quit so cancelDBusSubscription()
    if (m_subscriptionId != -1) {
        cancelDBusSubscription();
    }
}

bool KCupsConnection::readyToStart()
{
    if (QThread::currentThread() == this) {
        password_retries = 0;
        internalErrorCount = 0;
        return true;
    }
    return false;
}

ReturnArguments KCupsConnection::request(ipp_op_e operation,
                                         const char *resource,
                                         const QVariantHash &reqValues,
                                         bool needResponse)
{
    ReturnArguments ret;

    if (!readyToStart()) {
        return ret; // This is not intended to be used in the gui thread
    }

    ipp_t *response = NULL;
    bool needDestName = false;
    int group_tag = IPP_TAG_PRINTER;
    do {
        ipp_t *request;
        bool isClass = false;
        QString filename;
        QVariantHash values = reqValues;

        ippDelete(response);

        if (values.contains(QLatin1String("printer-is-class"))) {
            isClass = values.take(QLatin1String("printer-is-class")).toBool();
        }
        if (values.contains(QLatin1String("need-dest-name"))) {
            needDestName = values.take(QLatin1String("need-dest-name")).toBool();
        }
        if (values.contains(QLatin1String("group-tag-qt"))) {
            group_tag = values.take(QLatin1String("group-tag-qt")).toInt();
        }

        if (values.contains(QLatin1String("filename"))) {
            filename = values.take(QLatin1String("filename")).toString();
        }

        // Lets create the request
        if (values.contains(QLatin1String(KCUPS_PRINTER_NAME))) {
            request = ippNewDefaultRequest(values.take(QLatin1String(KCUPS_PRINTER_NAME)).toString(),
                                           isClass,
                                           operation);
        } else {
            request = ippNewRequest(operation);
        }

        // send our user name on the request too
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "requesting-user-name", NULL, cupsUser());

        // Add the requested values to the request
        requestAddValues(request, values);

        // Do the request
        // do the request deleting the response
        if (filename.isEmpty()) {
            response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, resource);
        } else {
            response = cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, resource, filename.toUtf8());
        }
    } while (retry(resource));

    if (response != NULL && needResponse) {
        ret = parseIPPVars(response, group_tag, needDestName);
    }
    ippDelete(response);

    return ret;
}

int KCupsConnection::renewDBusSubscription(int subscriptionId, int leaseDuration, const QStringList &events)
{
    int ret = -1;

    if (!readyToStart()) {
        kWarning() << "Tryied to run on the wrong thread";
        return subscriptionId; // This is not intended to be used in the gui thread
    }

    ipp_t *response = NULL;
    do {
        ipp_t *request;
        ipp_op_e operation;

        // check if we have a valid subscription ID
        if (subscriptionId >= 0) {
            // Add the "notify-events" values to the request
            operation = IPP_RENEW_SUBSCRIPTION;
        } else {
            operation = IPP_CREATE_PRINTER_SUBSCRIPTION;
        }

        // Lets create the request
        request = ippNewRequest(operation);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                     KCUPS_PRINTER_URI, NULL, "/");
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "requesting-user-name", NULL, cupsUser());

        if (operation == IPP_CREATE_PRINTER_SUBSCRIPTION) {
            // Add the "notify-events" values to the request
            QVariantHash values;
            values["notify-events"] = events;
            requestAddValues(request, values);

            ippAddString(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD,
                         "notify-pull-method", NULL, "ippget");
            ippAddString(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_URI,
                         "notify-recipient-uri", NULL, "dbus://");
            ippAddInteger(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                          "notify-lease-duration", leaseDuration);
        } else {
            ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                          "notify-subscription-id", subscriptionId);
            ippAddInteger(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER,
                          "notify-lease-duration", leaseDuration);
        }

        // Do the request
        response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");
    } while (retry("/"));

#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6
    if (response && ippGetStatusCode(response) == IPP_OK) {
#else
    if (response && response->request.status.status_code == IPP_OK) {
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6
        ipp_attribute_t *attr;
        if (subscriptionId >= 0) {
            // Request was ok, just return the current subscription
            ret = subscriptionId;
        } else if ((attr = ippFindAttribute(response,
                                            "notify-subscription-id",
                                            IPP_TAG_INTEGER)) == NULL) {
            kWarning() << "No notify-subscription-id in response!";
            ret = -1;
        } else {
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6
            ret = ippGetInteger(attr, 0);
        }
    } else if (subscriptionId >= 0 && response && ippGetStatusCode(response) == IPP_NOT_FOUND) {
        kDebug() << "Subscription not found";
        // When the subscription is not found try to get a new one
        return renewDBusSubscription(-1, leaseDuration, events);
#else
            ret = attr->values[0].integer;
        }
    } else if (subscriptionId >= 0 && response && response->request.status.status_code == IPP_NOT_FOUND) {
        kDebug() << "Subscription not found";
        // When the subscription is not found try to get a new one
        return renewDBusSubscription(-1, leaseDuration, events);
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6
    } else {
        kDebug() << "Request failed" << cupsLastError() << httpGetStatus(CUPS_HTTP_DEFAULT);
        // When the server stops/restarts we will have some error so ignore it
        ret = subscriptionId;
    }

    ippDelete(response);

    return ret;
}

void KCupsConnection::notifierConnect(const QString &signal, QObject *receiver, const char *slot)
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    systemBus.connect(QString(),
                      QLatin1String("/org/cups/cupsd/Notifier"),
                      QLatin1String("org.cups.cupsd.Notifier"),
                      signal,
                      receiver,
                      slot);
}

void KCupsConnection::connectNotify(const char *signal)
{
    QString event = eventForSignal(signal);
    if (!event.isNull()) {
        m_connectedEvents << event;
        QMetaObject::invokeMethod(m_subscriptionTimer,
                                  "start",
                                  Qt::QueuedConnection);
    }
}

void KCupsConnection::disconnectNotify(const char *signal)
{
    QString event = eventForSignal(signal);
    if (!event.isNull()) {
        m_connectedEvents.removeOne(event);
        QMetaObject::invokeMethod(m_subscriptionTimer,
                                  "start",
                                  Qt::QueuedConnection);
    }
}

QString KCupsConnection::eventForSignal(const char *signal) const
{
    // Server signals
    if (QLatin1String(signal) == SIGNAL(serverAudit(QString))) {
        return DBUS_SERVER_AUDIT;
    }
    if (QLatin1String(signal) == SIGNAL(serverStarted(QString))) {
        return DBUS_SERVER_STARTED;
    }
    if (QLatin1String(signal) == SIGNAL(serverStopped(QString))) {
        return DBUS_SERVER_STOPPED;
    }
    if (QLatin1String(signal) == SIGNAL(serverRestarted(QString))) {
        return DBUS_SERVER_RESTARTED;
    }

    // Printer signals
    if (QLatin1String(signal) == SIGNAL(printerAdded(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_ADDED;
    }
    if (QLatin1String(signal) == SIGNAL(printerDeleted(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_DELETED;
    }
    if (QLatin1String(signal) == SIGNAL(printerFinishingsChanged(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_FINISHINGS_CHANGED;
    }
    if (QLatin1String(signal) == SIGNAL(printerMediaChanged(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_MEDIA_CHANGED;
    }
    if (QLatin1String(signal) == SIGNAL(printerModified(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_MODIFIED;
    }
    if (QLatin1String(signal) == SIGNAL(printerRestarted(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_RESTARTED;
    }
    if (QLatin1String(signal) == SIGNAL(printerShutdown(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_SHUTDOWN;
    }
    if (QLatin1String(signal) == SIGNAL(printerStateChanged(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_STATE_CHANGED;
    }
    if (QLatin1String(signal) == SIGNAL(printerStopped(QString,QString,QString,uint,QString,bool))) {
        return DBUS_PRINTER_STOPPED;
    }

    // job signals
    if (QLatin1String(signal) == SIGNAL(jobCompleted(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint))) {
        return DBUS_JOB_COMPLETED;
    }
    if (QLatin1String(signal) == SIGNAL(jobConfigChanged(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint))) {
        return DBUS_JOB_CONFIG_CHANGED;
    }
    if (QLatin1String(signal) == SIGNAL(jobCreated(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint))) {
        return DBUS_JOB_CREATED;
    }
    if (QLatin1String(signal) == SIGNAL(jobProgress(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint))) {
        return DBUS_JOB_PROGRESS;
    }
    if (QLatin1String(signal) == SIGNAL(jobState(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint))) {
        return DBUS_JOB_STATE_CHANGED;
    }
    if (QLatin1String(signal) == SIGNAL(jobStopped(QString,QString,QString,uint,QString,bool,uint,uint,QString,QString,uint))) {
        return DBUS_JOB_STOPPED;
    }

    // No registered event signal matched
    return QString();
}

void KCupsConnection::updateSubscription()
{
    // Build the current list
    QStringList currentEvents = m_connectedEvents;
    currentEvents.sort();
    currentEvents.removeDuplicates();

    // Check if the requested events are already being asked
    if (m_requestedDBusEvents != currentEvents) {
        m_requestedDBusEvents = currentEvents;

        // If we alread have a subscription lets cancel
        // and create a new one
        if (m_subscriptionId >= 0) {
            cancelDBusSubscription();
        }

        // Canculates the new events
        renewDBusSubscription();
    }
}

void KCupsConnection::renewDBusSubscription()
{
    // check if we have a valid subscription ID
    if (m_subscriptionId >= 0) {
        m_subscriptionId = renewDBusSubscription(m_subscriptionId, SUBSCRIPTION_DURATION);
    }

    // The above request might fail if the subscription was cancelled
    if (m_subscriptionId < 0) {
        if (m_requestedDBusEvents.isEmpty()) {
            m_renewTimer->stop();
        } else {
            m_subscriptionId = renewDBusSubscription(m_subscriptionId, SUBSCRIPTION_DURATION, m_requestedDBusEvents);
            m_renewTimer->start();
        }
    }
}

void KCupsConnection::cancelDBusSubscription()
{
    do {
        ipp_t *request;

        // Lets create the request
        request = ippNewRequest(IPP_CANCEL_SUBSCRIPTION);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                     KCUPS_PRINTER_URI, NULL, "/");
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "requesting-user-name", NULL, cupsUser());
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "notify-subscription-id", m_subscriptionId);

        // Do the request
        ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/"));
    } while (retry("/"));

    // Reset the subscription id
    m_subscriptionId = -1;
}

void KCupsConnection::requestAddValues(ipp_t *request, const QVariantHash &values)
{
    QVariantHash::const_iterator i = values.constBegin();
    while (i != values.constEnd()) {
        QString key = i.key();
        QVariant value = i.value();
        switch (value.type()) {
        case QVariant::Bool:
            if (key == QLatin1String("printer-is-accepting-jobs")) {
                ippAddBoolean(request, IPP_TAG_PRINTER,
                              "printer-is-accepting-jobs", value.toBool());
            } else {
                ippAddBoolean(request, IPP_TAG_OPERATION,
                              key.toUtf8(), value.toBool());
            }
            break;
        case QVariant::Int:
            if (key == QLatin1String(KCUPS_JOB_ID)) {
                ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                              KCUPS_JOB_ID, value.toInt());
            } else if (key == QLatin1String(KCUPS_PRINTER_STATE)) {
                ippAddInteger(request, IPP_TAG_PRINTER, IPP_TAG_ENUM,
                              KCUPS_PRINTER_STATE, IPP_PRINTER_IDLE);
            } else {
                ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_ENUM,
                              key.toUtf8(), value.toInt());
            }
            break;
        case QVariant::String:
            if (key == QLatin1String(KCUPS_DEVICE_URI)) {
                // device uri has a different TAG
                ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                             KCUPS_DEVICE_URI, "utf-8", value.toString().toUtf8());
            } else if (key == QLatin1String("job-printer-uri")) {
                // TODO this seems broken
                const char* dest_name = value.toString().toUtf8();
                char  destUri[HTTP_MAX_URI];
                httpAssembleURIf(HTTP_URI_CODING_ALL, destUri, sizeof(destUri),
                                 "ipp", "utf-8", "localhost", ippPort(),
                                 "/printers/%s", dest_name);
                ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                             KCUPS_JOB_PRINTER_URI, "utf-8", value.toString().toUtf8());
            } else if (key == QLatin1String(KCUPS_PRINTER_OP_POLICY) ||
                       key == QLatin1String(KCUPS_PRINTER_ERROR_POLICY) ||
                       key == QLatin1String("ppd-name")) {
                // printer-op-policy has a different TAG
                ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                            key.toUtf8(), "utf-8", value.toString().toUtf8());
            } else if (key == QLatin1String(KCUPS_JOB_NAME)) {
                ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                             KCUPS_JOB_NAME, "utf-8", value.toString().toUtf8());
            } else if (key == QLatin1String("which-jobs")) {
                ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                             "which-jobs", "utf-8", value.toString().toUtf8());
            } else {
                ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_TEXT,
                             key.toUtf8(), "utf-8", value.toString().toUtf8());
            }
            break;
        case QVariant::StringList:
            {
                QStringList list = value.value<QStringList>();

                QList<QByteArray> valuesQByteArrayList;
                const char **values = qStringListToCharPtrPtr(list, &valuesQByteArrayList);

                if (key == QLatin1String(KCUPS_MEMBER_URIS)) {
                    ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                                  KCUPS_MEMBER_URIS, list.size(), "utf-8", values);
                } else if (key == QLatin1String("requested-attributes")) {
                    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                                  "requested-attributes", list.size(), "utf-8", values);
                } else if (key == QLatin1String("notify-events")) {
                    // Used for DBus notification, the values contains
                    // what we want to watch
                    ippAddStrings(request, IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD,
                                  "notify-events", list.size(), NULL, values);
                } else {
                    ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                                  i.key().toUtf8(), list.size(), "utf-8", values);
                }

                // ippAddStrings deep copies everything so we can throw away the values.
                // the QBAList and content is auto discarded when going out of scope.
                delete [] values;
            }
            break;
        case QVariant::UInt:
            ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_ENUM,
                          key.toUtf8(), value.toInt());
            break;
        default:
            kWarning() << "type NOT recognized! This will be ignored:" << key << "values" << value;
        }
        ++i;
    }
}

ReturnArguments KCupsConnection::parseIPPVars(ipp_t *response, int group_tag, bool needDestName)
{
    ipp_attribute_t *attr;
    ReturnArguments ret;

#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6
    Q_UNUSED(needDestName)

    QVariantHash destAttributes;
    for (attr = ippFirstAttribute(response); attr != NULL; attr = ippNextAttribute(response)) {
        // We hit an attribute sepparator
        if (ippGetName(attr) == NULL) {
            ret << destAttributes;
            destAttributes.clear();
            continue;
        }

        // Skip leading attributes until we hit a a group which can be a printer, job...
        if (ippGetGroupTag(attr) != group_tag ||
                (ippGetValueTag(attr) != IPP_TAG_INTEGER &&
                 ippGetValueTag(attr) != IPP_TAG_ENUM &&
                 ippGetValueTag(attr) != IPP_TAG_BOOLEAN &&
                 ippGetValueTag(attr) != IPP_TAG_TEXT &&
                 ippGetValueTag(attr) != IPP_TAG_TEXTLANG &&
                 ippGetValueTag(attr) != IPP_TAG_LANGUAGE &&
                 ippGetValueTag(attr) != IPP_TAG_NAME &&
                 ippGetValueTag(attr) != IPP_TAG_NAMELANG &&
                 ippGetValueTag(attr) != IPP_TAG_KEYWORD &&
                 ippGetValueTag(attr) != IPP_TAG_RANGE &&
                 ippGetValueTag(attr) != IPP_TAG_URI)) {
            continue;
        }

        // Add a printer description attribute...
        destAttributes[QString::fromUtf8(ippGetName(attr))] = ippAttrToVariant(attr);
    }

    if (!destAttributes.isEmpty()) {
        ret << destAttributes;
    }
#else
    for (attr = response->attrs; attr != NULL; attr = attr->next) {
       /*
        * Skip leading attributes until we hit a a group which can be a printer, job...
        */
        while (attr && attr->group_tag != group_tag) {
            attr = attr->next;
        }

        if (attr == NULL) {
            break;
        }

        /*
         * Pull the needed attributes from this printer...
         */
        QVariantHash destAttributes;
        for (; attr && attr->group_tag == group_tag; attr = attr->next) {
            if (attr->value_tag != IPP_TAG_INTEGER &&
                attr->value_tag != IPP_TAG_ENUM &&
                attr->value_tag != IPP_TAG_BOOLEAN &&
                attr->value_tag != IPP_TAG_TEXT &&
                attr->value_tag != IPP_TAG_TEXTLANG &&
                attr->value_tag != IPP_TAG_LANGUAGE &&
                attr->value_tag != IPP_TAG_NAME &&
                attr->value_tag != IPP_TAG_NAMELANG &&
                attr->value_tag != IPP_TAG_KEYWORD &&
                attr->value_tag != IPP_TAG_RANGE &&
                attr->value_tag != IPP_TAG_URI) {
                continue;
            }

            /*
             * Add a printer description attribute...
             */
            destAttributes[QString::fromUtf8(attr->name)] = ippAttrToVariant(attr);
        }

        /*
         * See if we have everything needed...
         */
        if (needDestName && destAttributes[QLatin1String(KCUPS_PRINTER_NAME)].toString().isEmpty()) {
            if (attr == NULL) {
                break;
            } else {
                continue;
            }
        }

        ret << destAttributes;

        if (attr == NULL) {
            break;
        }
    }
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6

    return ret;
}

// Don't forget to delete the request
ipp_t* KCupsConnection::ippNewDefaultRequest(const QString &name, bool isClass, ipp_op_t operation)
{
    char  uri[HTTP_MAX_URI]; // printer URI
    ipp_t *request;

    QString destination;
    if (isClass) {
        destination = QLatin1String("/classes/") % name;
    } else {
        destination = QLatin1String("/printers/") % name;
    }

    // Create a new request
    // where we need:
    // * printer-uri
    // * requesting-user-name
    request = ippNewRequest(operation);
    httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", "utf-8", "localhost",
                     ippPort(), destination.toUtf8());
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_PRINTER_URI,
                 "utf-8", uri);

    return request;
}

QVariant KCupsConnection::ippAttrToVariant(ipp_attribute_t *attr)
{
    QVariant ret;
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6
    switch (ippGetValueTag(attr)) {
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
        if (ippGetCount(attr) == 1) {
            ret = ippGetInteger(attr, 0);
        } else {
            QList<int> values;
            for (int i = 0; i < ippGetCount(attr); ++i) {
                values << ippGetInteger(attr, i);
            }
            ret = qVariantFromValue(values);
        }
        break;
    case IPP_TAG_BOOLEAN:
        if (ippGetCount(attr)== 1) {
            ret = ippGetBoolean(attr, 0);
        } else {
            QList<bool> values;
            for (int i = 0; i < ippGetCount(attr); ++i) {
                values << ippGetBoolean(attr, i);
            }
            ret = qVariantFromValue(values);
        }
        break;
    case IPP_TAG_RANGE:
    {
        QVariantList values;
        for (int i = 0; i < ippGetCount(attr); ++i) {
            int rangeUpper;
            values << ippGetRange(attr, i, &rangeUpper);
            values << rangeUpper;
        }
        ret = values;
    }
        break;
    default:
        if (ippGetCount(attr)== 1) {
            ret = QString::fromUtf8(ippGetString(attr, 0, NULL));
        } else {
            QStringList values;
            for (int i = 0; i < ippGetCount(attr); ++i) {
                values << QString::fromUtf8(ippGetString(attr, i, NULL));
            }
            ret = values;
        }
    }
#else
    switch (attr->value_tag) {
    case IPP_TAG_INTEGER:
    case IPP_TAG_ENUM:
        if (attr->num_values == 1) {
            ret = attr->values[0].integer;
        } else {
            QList<int> values;
            for (int i = 0; i < attr->num_values; ++i) {
                values << attr->values[i].integer;
            }
            ret = qVariantFromValue(values);
        }
        break;
    case IPP_TAG_BOOLEAN:
        if (attr->num_values == 1) {
            ret = static_cast<bool>(attr->values[0].integer);
        } else {
            QList<bool> values;
            for (int i = 0; i < attr->num_values; ++i) {
                values << static_cast<bool>(attr->values[i].integer);
            }
            ret = qVariantFromValue(values);
        }
        break;
    case IPP_TAG_RANGE:
    {
        QVariantList values;
        for (int i = 0; i < attr->num_values; ++i) {
            values << attr->values[i].range.lower;
            values << attr->values[i].range.upper;
        }
        ret = values;
    }
        break;
    default:
        if (attr->num_values == 1) {
            ret = QString::fromUtf8(attr->values[0].string.text);
        } else {
            QStringList values;
            for (int i = 0; i < attr->num_values; ++i) {
                values << QString::fromUtf8(attr->values[i].string.text);
            }
            ret = values;
        }
    }
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 6
    return ret;
}

bool KCupsConnection::retry(const char *resource)
{
    ipp_status_t status = cupsLastError();

    kDebug() << "cupsLastError():" << status << cupsLastErrorString();

    // When CUPS process stops our connection
    // with it fails and has to be re-established
    if (status == IPP_INTERNAL_ERROR) {
        // Deleting this connection thread forces it
        // to create a new CUPS connection
        kWarning() << "IPP_INTERNAL_ERROR: clearing cookies and reconnecting";

        // TODO maybe reconnect is enough
//        httpClearCookie(CUPS_HTTP_DEFAULT);

        // Reconnect to CUPS
        if (httpReconnect(CUPS_HTTP_DEFAULT)) {
            kWarning() << "IPP_INTERNAL_ERROR: failed to reconnect";
            // Server might be restarting sleep for a few ms
            msleep(500);
        }

        // Try the request again
        return ++internalErrorCount < 3;
    }

    bool forceAuth = false;
    // If our user is forbidden to perform the
    // task we try again using the root user
    // ONLY if it was the first time
    if (status == IPP_FORBIDDEN &&
        password_retries == 0) {
        // Pretend to be the root user
        // Sometimes setting this just works
        cupsSetUser("root");

        // force authentication
        forceAuth = true;
    }

    if (status == IPP_NOT_AUTHORIZED ||
        status == IPP_NOT_AUTHENTICATED) {
        if (password_retries > 3 || password_retries == -1) {
            // the authentication failed 3 times
            // OR the dialog was canceld (-1)
            // reset to 0 and quit the do-while loop
            password_retries = 0;
            return false;
        }

        // force authentication
        forceAuth = true;
    }

    if (forceAuth) {
        // force authentication
        kDebug() << "cupsDoAuthentication() password_retries:" << password_retries;
        int ret = cupsDoAuthentication(CUPS_HTTP_DEFAULT, "POST", resource);
        kDebug() << "cupsDoAuthentication() success:" << (ret == -1 ? true : false);

        // If the authentication was succefull
        // sometimes just trying to be root works
        return ret == -1 ? true : false;
    }

    // the action was not forbidden
    return false;
}

const char * password_cb(const char *prompt, http_t *http, const char *method, const char *resource, void *user_data)
{
    Q_UNUSED(prompt)
    Q_UNUSED(http)
    Q_UNUSED(method)
    Q_UNUSED(resource)

    if (++password_retries > 3) {
        // cancel the authentication
        cupsSetUser(NULL);
        return NULL;
    }

    KCupsPasswordDialog *passwordDialog = static_cast<KCupsPasswordDialog *>(user_data);
    bool wrongPassword = password_retries > 1;

    // This will block this thread until exec is not finished
    kDebug() << password_retries;
    QMetaObject::invokeMethod(passwordDialog,
                              "exec",
                              Qt::BlockingQueuedConnection,
                              Q_ARG(QString, QString::fromUtf8(cupsUser())),
                              Q_ARG(bool, wrongPassword));
    kDebug() << passwordDialog->accepted();

    // The password dialog has just returned check the result
    // method that returns QDialog enums
    if (passwordDialog->accepted()) {
        cupsSetUser(passwordDialog->username().toUtf8());
        return passwordDialog->password().toUtf8();
    } else {
        // the dialog was canceled
        password_retries = -1;
        cupsSetUser(NULL);
        return NULL;
    }
}
