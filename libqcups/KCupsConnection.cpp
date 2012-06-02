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

#include <QCoreApplication>
#include <QStringBuilder>
#include <QDBusConnection>
#include <QByteArray>

#include <KLocale>
#include <KDebug>

#include <cups/cups.h>

#define RENEW_INTERVAL        3500
#define SUBSCRIPTION_DURATION 3600

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<bool>)

KCupsConnection* KCupsConnection::m_instance = 0;
static int password_retries = 0;
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
    QThread(parent),
    m_inited(false),
    // Creating the dialog before start() will make it run on the gui thread
    m_passwordDialog(new KPasswordDialog(0L, KPasswordDialog::ShowUsernameLine)),
    m_subscriptionId(-1)
{
    m_passwordDialog->setModal(true);
    m_passwordDialog->setPrompt(i18n("Enter an username and a password to complete the task"));

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

    // Starts this thread
    start();
}

KCupsConnection::~KCupsConnection()
{
    m_instance = 0;
    m_renewTimer->deleteLater();

    quit();
    wait();
}

void KCupsConnection::run()
{
    // This is dead cool, cups will call the thread_password_cb()
    // function when a password set is needed, as we passed the
    // password dialog pointer the functions just need to call
    // it on a blocking mode.
    cupsSetPasswordCB2(password_cb, m_passwordDialog);

    // Creates the timer that will renew the DBus subscription
    m_renewTimer = new QTimer;
    m_renewTimer->setInterval(RENEW_INTERVAL);
    connect(m_renewTimer, SIGNAL(timeout()), this, SLOT(renewDBusSubscription()));

    m_inited = true;
    exec();
}

bool KCupsConnection::readyToStart()
{
    if (QThread::currentThread() == KCupsConnection::global()) {
        password_retries = 0;
        return true;
    }
    return false;
}

ReturnArguments KCupsConnection::request(ipp_op_e operation,
                                         const QString &resource,
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
        if (values.contains(QLatin1String("printer-name"))) {
            request = ippNewDefaultRequest(values.take(QLatin1String("printer-name")).toString(),
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
            response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, resource.toUtf8());
        } else {
            response = cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, resource.toUtf8(), filename.toUtf8());
        }
    } while (retryIfForbidden());

    if (response != NULL && needResponse) {
        ret = parseIPPVars(response, group_tag, needDestName);
    }
    ippDelete(response);

    return ret;
}

int KCupsConnection::createDBusSubscription(const QStringList &events)
{
    // Build the current list
    QStringList currentEvents;
    foreach (const QStringList &registeredEvents, m_requestedDBusEvents) {
        currentEvents << registeredEvents;
    }
    currentEvents.removeDuplicates();

    // Check if the requested events are already being asked
    bool equal = true;
    foreach (const QString &event, events) {
        if (!currentEvents.contains(event)) {
            equal = false;
        }
    }

    // Store the subscription
    int id = 1;
    if (!m_requestedDBusEvents.isEmpty()) {
        id = m_requestedDBusEvents.keys().last();
        ++id;
    }
    m_requestedDBusEvents[id] = events;

    // If the requested list is included in our request just
    // return an ID
    if (equal) {
        return id;
    }

    // If we alread have a subscription lets cancel
    // and create a new one
    if (m_subscriptionId >= 0) {
        cancelDBusSubscription();
    }

    currentEvents << events;

    // Canculates the new events
    renewDBusSubscription();

    return id;
}

void KCupsConnection::removeDBusSubscription(int subscriptionId)
{
    // Collect the list of current events
    QStringList currentEvents;
    foreach (const QStringList &registeredEvents, m_requestedDBusEvents) {
        currentEvents << registeredEvents;
    }
    currentEvents.removeDuplicates();

    QStringList removedEvents = m_requestedDBusEvents.take(subscriptionId);

    // Check if the removed events list is the same as the list we
    // need, if yes means we can keep renewing the same events
    if (removedEvents == currentEvents && !m_requestedDBusEvents.isEmpty()) {
        return;
    } else {
        // The requested events changed
        cancelDBusSubscription();

        // Canculates the new events
        renewDBusSubscription();
    }
}

int KCupsConnection::renewDBusSubscription(int subscriptionId, int leaseDuration, const QStringList &events)
{
    int ret = -1;

    if (!readyToStart()) {
        return subscriptionId; // This is not intended to be used in the gui thread
    }

    ipp_t *response = NULL;
    do {
        ipp_t *request;
        ipp_op_e operation;
        ret = subscriptionId;

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
                     "printer-uri", NULL, "/");
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
    } while (retryIfForbidden());

    if (ret < 0 &&
        response != NULL &&
        response->request.status.status_code <= IPP_OK_CONFLICT) {
        ipp_attribute_t *attr = NULL;
        if ((attr = ippFindAttribute(response, "notify-subscription-id",
                                     IPP_TAG_INTEGER)) == NULL) {
            kDebug() << "No notify-subscription-id in response!";
        } else {
            ret = attr->values[0].integer;
        }
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

void KCupsConnection::cancelDBusSubscription()
{
    do {
        ipp_t *request;

        // Lets create the request
        request = ippNewRequest(IPP_CANCEL_SUBSCRIPTION);
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                     "printer-uri", NULL, "/");
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                     "requesting-user-name", NULL, cupsUser());
        ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                      "notify-subscription-id", m_subscriptionId);

        // Do the request
        ippDelete(cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/"));
    } while (retryIfForbidden());

    // Reset the subscription id
    m_subscriptionId = -1;
}

void KCupsConnection::renewDBusSubscription()
{
    // check if we have a valid subscription ID
    if (m_subscriptionId >= 0) {
        kDebug() << m_subscriptionId;
        renewDBusSubscription(m_subscriptionId, SUBSCRIPTION_DURATION);
    } else {
        QStringList currentEvents;
        foreach (const QStringList &registeredEvents, m_requestedDBusEvents) {
            currentEvents << registeredEvents;
        }
        currentEvents.removeDuplicates();
        kDebug() << currentEvents;

        if (!currentEvents.isEmpty()) {
            m_subscriptionId = renewDBusSubscription(m_subscriptionId, SUBSCRIPTION_DURATION, currentEvents);
            m_renewTimer->start();
        } else {
            m_renewTimer->stop();
        }
    }
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
            if (key == QLatin1String("job-id")) {
                ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_INTEGER,
                              "job-id", value.toInt());
            } else if (key == QLatin1String("printer-state")) {
                ippAddInteger(request, IPP_TAG_PRINTER, IPP_TAG_ENUM,
                              "printer-state", IPP_PRINTER_IDLE);
            } else {
                ippAddInteger(request, IPP_TAG_OPERATION, IPP_TAG_ENUM,
                              key.toUtf8(), value.toInt());
            }
            break;
        case QVariant::String:
            if (key == QLatin1String("device-uri")) {
                // device uri has a different TAG
                ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                            "device-uri", "utf-8", value.toString().toUtf8());
            } else if (key == QLatin1String("job-printer-uri")) {
                // TODO this seems broken
                const char* dest_name = value.toString().toUtf8();
                char  destUri[HTTP_MAX_URI];
                httpAssembleURIf(HTTP_URI_CODING_ALL, destUri, sizeof(destUri),
                                 "ipp", "utf-8", "localhost", ippPort(),
                                 "/printers/%s", dest_name);
                ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                             "job-printer-uri", "utf-8", value.toString().toUtf8());
            } else if (key == QLatin1String("printer-op-policy") ||
                       key == QLatin1String("printer-error-policy") ||
                       key == QLatin1String("ppd-name")) {
                // printer-op-policy has a different TAG
                ippAddString(request, IPP_TAG_PRINTER, IPP_TAG_NAME,
                            key.toUtf8(), "utf-8", value.toString().toUtf8());
            } else if (key == QLatin1String("job-name")) {
                ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME,
                             "job-name", "utf-8", value.toString().toUtf8());
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

                if (key == QLatin1String("member-uris")) {
                    ippAddStrings(request, IPP_TAG_PRINTER, IPP_TAG_URI,
                                  "member-uris", list.size(), "utf-8", values);
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
        if (needDestName && destAttributes[QLatin1String("printer-name")].toString().isEmpty()) {
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
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                 "utf-8", uri);

    return request;
}

QVariant KCupsConnection::ippAttrToVariant(ipp_attribute_t *attr)
{
    if (attr->num_values == 1 &&
        attr->value_tag != IPP_TAG_INTEGER &&
        attr->value_tag != IPP_TAG_ENUM &&
        attr->value_tag != IPP_TAG_BOOLEAN &&
        attr->value_tag != IPP_TAG_RANGE) {
        return QString::fromUtf8(attr->values[0].string.text);
    }

    if (attr->value_tag == IPP_TAG_INTEGER || attr->value_tag == IPP_TAG_ENUM) {
        if (attr->num_values == 1) {
            return attr->values[0].integer;
        } else {
            QList<int> values;
            for (int i = 0; i < attr->num_values; ++i) {
                values << attr->values[i].integer;
            }
            return QVariant::fromValue(values);
        }
    } else if (attr->value_tag == IPP_TAG_BOOLEAN) {
        if (attr->num_values == 1) {
            return static_cast<bool>(attr->values[0].integer);
        } else {
            QList<bool> values;
            for (int i = 0; i < attr->num_values; ++i) {
                values << static_cast<bool>(attr->values[i].integer);
            }
            return QVariant::fromValue(values);
        }
    } else if (attr->value_tag == IPP_TAG_RANGE) {
        QVariantList values;
        for (int i = 0; i < attr->num_values; ++i) {
            values << attr->values[i].range.lower;
            values << attr->values[i].range.upper;
        }
        return values;
    } else {
        QStringList values;
        for (int i = 0; i < attr->num_values; ++i) {
            values << QString::fromUtf8(attr->values[i].string.text);
        }
        return values;
    }
}

bool KCupsConnection::retryIfForbidden()
{
    ipp_status_t status = cupsLastError();
    if (status == IPP_FORBIDDEN ||
        status == IPP_NOT_AUTHORIZED ||
        status == IPP_NOT_AUTHENTICATED) {
        if (password_retries == 0) {
            // Pretend to be the root user
            // Sometime seting this just works
            cupsSetUser("root");
        } else if (password_retries > 3 || password_retries == -1) {
            // the authentication failed 3 times
            // OR the dialog was canceld (-1)
            // reset to 0 and quit the do-while loop
            password_retries = 0;
            return false;
        }

        // force authentication
        kDebug() << "cupsLastErrorString()" << cupsLastErrorString() << status;
        kDebug() << "cupsDoAuthentication" << password_retries;
        cupsDoAuthentication(CUPS_HTTP_DEFAULT, "POST", "/");
        // tries to do the action again
        // sometimes just trying to be root works
        return true;
    }

    // the action was not forbidden
    return false;
}

ipp_status_t KCupsConnection::lastError()
{
    ipp_status_t status = cupsLastError();

    // When CUPS process stops our connection
    // with it fails and has to be re-established
    if (status == IPP_INTERNAL_ERROR) {
        // Deleting this connection thread forces it
        // to create a new CUPS_HTTP_DEFAULT connection
        KCupsConnection::global()->deleteLater();
    }
    return status;

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

    KPasswordDialog *passwordDialog = static_cast<KPasswordDialog *>(user_data);
    passwordDialog->setUsername(QString::fromUtf8(cupsUser()));
    if (password_retries > 1) {
        passwordDialog->showErrorMessage(QString(), KPasswordDialog::UsernameError);
        passwordDialog->showErrorMessage(i18n("Wrong username or password"), KPasswordDialog::PasswordError);
    }

    // This will block this thread until exec is not finished
    QMetaObject::invokeMethod(passwordDialog,
                              "exec",
                              Qt::BlockingQueuedConnection);

    if (passwordDialog->result() == KDialog::Ok) {
        cupsSetUser(passwordDialog->username().toUtf8());
        return passwordDialog->password().toUtf8();
    } else {
        // the dialog was canceled
        password_retries = -1;
        cupsSetUser(NULL);
        return NULL;
    }
}
