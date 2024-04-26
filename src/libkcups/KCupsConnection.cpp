/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2012 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsConnection.h"
#include "KCupsPasswordDialog.h"
#include "KIppRequest.h"
#include "kcupslib_log.h"

#include <QCoreApplication>
#include <QDBusConnection>

#include <KLocalizedString>

using namespace Qt::StringLiterals;

KCupsConnection *KCupsConnection::s_instance = nullptr;
static int SUBSCRIPTION_DURATION = 3600;
static int password_retries = 0;
static int total_retries = 0;
static int internalErrorCount = 0;
const char *password_cb(const char *prompt, http_t *http, const char *method, const char *resource, void *user_data);

KCupsConnection *KCupsConnection::global()
{
    if (!s_instance) {
        setup();
    }
    return s_instance;
}

void KCupsConnection::setup(NotifySubscriptions flags)
{
    if (!s_instance) {
        s_instance = new KCupsConnection(flags, qApp);
    } else {
        if (flags != s_instance->m_subscriptions) {
            s_instance->m_renewTimer.stop();
            s_instance->cancelDBusSubscription();
            s_instance->m_requestedDBusEvents.clear();
            s_instance->m_subscriptions = flags;
            s_instance->setupDbusSubscriptions();
            s_instance->renewDBusSubscription(s_instance->m_subscriptionId, SUBSCRIPTION_DURATION, s_instance->m_requestedDBusEvents);
            s_instance->m_renewTimer.start();
        }
    }
}

KCupsConnection::KCupsConnection(NotifySubscriptions flags, QObject *parent)
    : QThread(parent)
{
    if (!flags) {
        m_subscriptions = NotifyType::Printers | NotifyType::Server | NotifyType::Jobs;
    } else {
        m_subscriptions = flags;
    }
    qCDebug(LIBKCUPS) << "CUPS Connection created with" << m_subscriptions;
    init();
}

KCupsConnection::KCupsConnection(const QUrl &server, QObject *parent)
    : QThread(parent)
    , m_serverUrl(server)
{
    qRegisterMetaType<KIppRequest>("KIppRequest");
    init();
}

KCupsConnection::~KCupsConnection()
{
    if (m_subscriptionId >= 0) {
        cancelDBusSubscription();
    }

    if (s_instance == this) {
        s_instance = nullptr;
    }
    m_passwordDialog->deleteLater();

    quit();
    wait();
}

void KCupsConnection::setPasswordMainWindow(WId mainwindow)
{
    m_passwordDialog->setMainWindow(mainwindow);
}

void KCupsConnection::init()
{
    // Creating the dialog before start() will make it run on the gui thread
    m_passwordDialog = new KCupsPasswordDialog;

    setupDbusSubscriptions();

    // Dbus notifier subscription renewal
    m_renewTimer.setInterval(3500 * 1000);
    connect(&m_renewTimer, &QTimer::timeout, this, static_cast<void (KCupsConnection::*)()>(&KCupsConnection::renewDBusSubscription), Qt::AutoConnection);

    renewDBusSubscription();

    start();
}

void KCupsConnection::setupDbusSubscriptions()
{
    if (m_subscriptions & KCupsConnection::NotifyType::Server) {
        notifierConnect("ServerStarted"_L1, this, SIGNAL(serverStarted(QString)), "server-started"_L1);
        notifierConnect("ServerStopped"_L1, this, SIGNAL(serverStopped(QString)), "server-stopped"_L1);
        notifierConnect("ServerRestarted"_L1, this, SIGNAL(serverRestarted(QString)), "server-restarted"_L1);
        notifierConnect("ServerAudit"_L1, this, SIGNAL(serverAudit(QString)), "server-audit"_L1);
    }

    if (m_subscriptions & KCupsConnection::NotifyType::Printers) {
        notifierConnect("PrinterAdded"_L1, this, SIGNAL(printerAdded(QString, QString, QString, uint, QString, bool)), "printer-added"_L1);
        notifierConnect("PrinterModified"_L1, this, SIGNAL(printerModified(QString, QString, QString, uint, QString, bool)), "printer-modified"_L1);
        notifierConnect("PrinterDeleted"_L1, this, SIGNAL(printerDeleted(QString, QString, QString, uint, QString, bool)), "printer-deleted"_L1);
        notifierConnect("PrinterStateChanged"_L1, this, SIGNAL(printerStateChanged(QString, QString, QString, uint, QString, bool)), "printer-state-changed"_L1);
        notifierConnect("PrinterStopped"_L1, this, SIGNAL(printerStopped(QString, QString, QString, uint, QString, bool)), "printer-stopped"_L1);
        notifierConnect("PrinterShutdown"_L1, this, SIGNAL(printerShutdown(QString, QString, QString, uint, QString, bool)), "printer-shutdown"_L1);
        notifierConnect("PrinterRestarted"_L1, this, SIGNAL(printerRestarted(QString, QString, QString, uint, QString, bool)), "printer-restarted"_L1);
        notifierConnect("PrinterMediaChanged"_L1, this, SIGNAL(printerMediaChanged(QString, QString, QString, uint, QString, bool)), "printer-media-changed"_L1);
        // TODO: figure out if this is deprecated with 2.x
        notifierConnect("PrinterFinishingsChanged"_L1,
                        this,
                        SIGNAL(PrinterFinishingsChanged(QString, QString, QString, uint, QString, bool)),
                        "printer-finishings-changed"_L1);
    }

    if (m_subscriptions & KCupsConnection::NotifyType::Jobs) {
        notifierConnect("JobState"_L1,
                        this,
                        SIGNAL(jobState(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)),
                        "job-state-changed"_L1);
        notifierConnect("JobCreated"_L1,
                        this,
                        SIGNAL(jobCreated(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)),
                        "job-created"_L1);
        notifierConnect("JobStopped"_L1,
                        this,
                        SIGNAL(jobStopped(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)),
                        "job-stopped"_L1);
        notifierConnect("JobConfigChanged"_L1,
                        this,
                        SIGNAL(jobConfigChanged(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)),
                        "job-config-changed"_L1);
        notifierConnect("JobProgress"_L1,
                        this,
                        SIGNAL(jobProgress(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)),
                        "job-progress"_L1);
        notifierConnect("JobCompleted"_L1,
                        this,
                        SIGNAL(jobCompleted(QString, QString, QString, uint, QString, bool, uint, uint, QString, QString, uint)),
                        "job-completed"_L1);
    }
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

        cupsSetServer(qUtf8Printable(m_serverUrl.authority()));
    }

    // This is dead cool, cups will call the thread_password_cb()
    // function when a password set is needed, as we passed the
    // password dialog pointer the functions just need to call
    // it on a blocking mode.
    cupsSetPasswordCB2(password_cb, m_passwordDialog);

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
        total_retries = 0;
        internalErrorCount = 0;
        return true;
    }
    return false;
}

ReturnArguments KCupsConnection::request(const KIppRequest &request, ipp_tag_t groupTag) const
{
    ReturnArguments ret;
    ipp_t *response = nullptr;
    do {
        ippDelete(response);
        response = nullptr;

        response = request.sendIppRequest();
    } while (retry(qUtf8Printable(request.resource()), request.operation()));

    if (response && groupTag != IPP_TAG_ZERO) {
        ret = parseIPPVars(response, groupTag);
    }
    ippDelete(response);

    return ret;
}

int KCupsConnection::renewDBusSubscription(int subscriptionId, int leaseDuration, const QStringList &events)
{
    int ret = -1;

    ipp_op_t operation;

    // check if we have a valid subscription ID
    if (subscriptionId >= 0) {
        // Add the "notify-events" values to the request
        operation = IPP_RENEW_SUBSCRIPTION;
    } else {
        operation = IPP_CREATE_PRINTER_SUBSCRIPTION;
    }

    KIppRequest request(operation, "/"_L1);
    request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_PRINTER_URI, "/"_L1);
    request.addInteger(IPP_TAG_SUBSCRIPTION, IPP_TAG_INTEGER, KCUPS_NOTIFY_LEASE_DURATION, leaseDuration);

    if (operation == IPP_CREATE_PRINTER_SUBSCRIPTION) {
        // Add the "notify-events" values to the request
        request.addStringList(IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD, KCUPS_NOTIFY_EVENTS, events);
        request.addString(IPP_TAG_SUBSCRIPTION, IPP_TAG_KEYWORD, KCUPS_NOTIFY_PULL_METHOD, "ippget"_L1);
        request.addString(IPP_TAG_SUBSCRIPTION, IPP_TAG_URI, KCUPS_NOTIFY_RECIPIENT_URI, "dbus://"_L1);
        qCDebug(LIBKCUPS) << "SUBSCRIPTION:" << ippOpString(operation) << events;
    } else {
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_NOTIFY_SUBSCRIPTION_ID, subscriptionId);
        qCDebug(LIBKCUPS) << "SUBSCRIPTION:" << subscriptionId << ippOpString(operation);
    }

    ipp_t *response = nullptr;
    do {
        // Do the request
        response = request.sendIppRequest();
    } while (retry("/", operation));

#if !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    if (response && ippGetStatusCode(response) == IPP_OK) {
#else
    if (response && response->request.status.status_code == IPP_OK) {
#endif // !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
        ipp_attribute_t *attr;
        if (subscriptionId >= 0) {
            // Request was ok, just return the current subscription
            ret = subscriptionId;
        } else if ((attr = ippFindAttribute(response, "notify-subscription-id", IPP_TAG_INTEGER)) == nullptr) {
            qCWarning(LIBKCUPS) << "No notify-subscription-id in response!";
            ret = -1;
        } else {
#if !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
            ret = ippGetInteger(attr, 0);
        }
    } else if (subscriptionId >= 0 && response && ippGetStatusCode(response) == IPP_NOT_FOUND) {
        qCDebug(LIBKCUPS) << "Subscription not found";
        // When the subscription is not found try to get a new one
        return renewDBusSubscription(-1, leaseDuration, events);
#else
            ret = attr->values[0].integer;
        }
    } else if (subscriptionId >= 0 && response && response->request.status.status_code == IPP_NOT_FOUND) {
        qCDebug(LIBKCUPS) << "Subscription not found";
        // When the subscription is not found try to get a new one
        return renewDBusSubscription(-1, leaseDuration, events);
#endif // !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    } else {
        qCDebug(LIBKCUPS) << "Request failed" << cupsLastError() << httpGetStatus(CUPS_HTTP_DEFAULT);
        // When the server stops/restarts we will have some error so ignore it
        ret = subscriptionId;
    }

    ippDelete(response);

    return ret;
}

void KCupsConnection::notifierConnect(const QString &signal, QObject *receiver, const char *slot, const QString &sub)
{
    if (QDBusConnection::systemBus().connect(QString(), "/org/cups/cupsd/Notifier"_L1, "org.cups.cupsd.Notifier"_L1, signal, receiver, slot)) {
        m_requestedDBusEvents << sub;
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
            m_renewTimer.stop();
        } else {
            m_subscriptionId = renewDBusSubscription(m_subscriptionId, SUBSCRIPTION_DURATION, m_requestedDBusEvents);
            m_renewTimer.start();
        }
    }
}

void KCupsConnection::cancelDBusSubscription()
{
    KIppRequest request(IPP_CANCEL_SUBSCRIPTION, "/"_L1);
    request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_PRINTER_URI, "/"_L1);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_NOTIFY_SUBSCRIPTION_ID, m_subscriptionId);

    do {
        ippDelete(request.sendIppRequest());
    } while (retry(qUtf8Printable(request.resource()), request.operation()));

    m_subscriptionId = -1;
}

ReturnArguments KCupsConnection::parseIPPVars(ipp_t *response, ipp_tag_t group_tag)
{
    ipp_attribute_t *attr;
    ReturnArguments ret;

#if !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    QVariantMap destAttributes;
    for (attr = ippFirstAttribute(response); attr != nullptr; attr = ippNextAttribute(response)) {
        // We hit an attribute separator
        if (ippGetName(attr) == nullptr) {
            ret << destAttributes;
            destAttributes.clear();
            continue;
        }

        // Skip leading attributes until we hit a group which can be a printer, job...
        if (ippGetGroupTag(attr) != group_tag
            || (ippGetValueTag(attr) != IPP_TAG_INTEGER && ippGetValueTag(attr) != IPP_TAG_ENUM && ippGetValueTag(attr) != IPP_TAG_BOOLEAN
                && ippGetValueTag(attr) != IPP_TAG_TEXT && ippGetValueTag(attr) != IPP_TAG_TEXTLANG && ippGetValueTag(attr) != IPP_TAG_LANGUAGE
                && ippGetValueTag(attr) != IPP_TAG_NAME && ippGetValueTag(attr) != IPP_TAG_NAMELANG && ippGetValueTag(attr) != IPP_TAG_KEYWORD
                && ippGetValueTag(attr) != IPP_TAG_RANGE && ippGetValueTag(attr) != IPP_TAG_URI)) {
            continue;
        }

        // Add a printer description attribute...
        destAttributes[QString::fromUtf8(ippGetName(attr))] = ippAttrToVariant(attr);
    }

    if (!destAttributes.isEmpty()) {
        ret << destAttributes;
    }
#else
    for (attr = response->attrs; attr != nullptr; attr = attr->next) {
        /*
         * Skip leading attributes until we hit a group which can be a printer, job...
         */
        while (attr && attr->group_tag != group_tag) {
            attr = attr->next;
        }

        if (attr == nullptr) {
            break;
        }

        /*
         * Pull the needed attributes from this printer...
         */
        QVariantMap destAttributes;
        for (; attr && attr->group_tag == group_tag; attr = attr->next) {
            if (attr->value_tag != IPP_TAG_INTEGER && attr->value_tag != IPP_TAG_ENUM && attr->value_tag != IPP_TAG_BOOLEAN && attr->value_tag != IPP_TAG_TEXT
                && attr->value_tag != IPP_TAG_TEXTLANG && attr->value_tag != IPP_TAG_LANGUAGE && attr->value_tag != IPP_TAG_NAME
                && attr->value_tag != IPP_TAG_NAMELANG && attr->value_tag != IPP_TAG_KEYWORD && attr->value_tag != IPP_TAG_RANGE
                && attr->value_tag != IPP_TAG_URI) {
                continue;
            }

            /*
             * Add a printer description attribute...
             */
            destAttributes[QString::fromUtf8(attr->name)] = ippAttrToVariant(attr);
        }

        ret << destAttributes;

        if (attr == nullptr) {
            break;
        }
    }
#endif // !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)

    return ret;
}

QVariant KCupsConnection::ippAttrToVariant(ipp_attribute_t *attr)
{
    QVariant ret;
#if !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
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
            ret = QVariant::fromValue(values);
        }
        break;
    case IPP_TAG_BOOLEAN:
        if (ippGetCount(attr) == 1) {
            ret = ippGetBoolean(attr, 0);
        } else {
            QList<bool> values;
            for (int i = 0; i < ippGetCount(attr); ++i) {
                values << ippGetBoolean(attr, i);
            }
            ret = QVariant::fromValue(values);
        }
        break;
    case IPP_TAG_RANGE: {
        QVariantList values;
        for (int i = 0; i < ippGetCount(attr); ++i) {
            int rangeUpper;
            values << ippGetRange(attr, i, &rangeUpper);
            values << rangeUpper;
        }
        ret = values;
    } break;
    default:
        if (ippGetCount(attr) == 1) {
            ret = QString::fromUtf8(ippGetString(attr, 0, nullptr));
        } else {
            QStringList values;
            for (int i = 0; i < ippGetCount(attr); ++i) {
                values << QString::fromUtf8(ippGetString(attr, i, nullptr));
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
            ret = QVariant::fromValue(values);
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
            ret = QVariant::fromValue(values);
        }
        break;
    case IPP_TAG_RANGE: {
        QVariantList values;
        for (int i = 0; i < attr->num_values; ++i) {
            values << attr->values[i].range.lower;
            values << attr->values[i].range.upper;
        }
        ret = values;
    } break;
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
#endif // !(CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6)
    return ret;
}

bool KCupsConnection::retry(const char *resource, int operation) const
{
    ipp_status_t status = cupsLastError();

    if (operation != -1) {
        qCDebug(LIBKCUPS) << ippOpString(static_cast<ipp_op_t>(operation)) << "last error:" << status << cupsLastErrorString();
    } else {
        qCDebug(LIBKCUPS) << operation << "last error:" << status << cupsLastErrorString();
    }

    // When CUPS process stops our connection
    // with it fails and has to be re-established
    if (status == IPP_INTERNAL_ERROR) {
        // Deleting this connection thread forces it
        // to create a new CUPS connection
        qCWarning(LIBKCUPS) << "IPP_INTERNAL_ERROR: clearing cookies and reconnecting";

        // Reconnect to CUPS
        int cancel = 0;
        if (httpReconnect2(CUPS_HTTP_DEFAULT, 10000, &cancel)) {
            qCWarning(LIBKCUPS) << "Failed to reconnect" << cupsLastErrorString();
        }

        // Try the request again
        return ++internalErrorCount < 3;
    }

    total_retries++;

    if (total_retries > (password_retries + 3)) {
        // Something is wrong.
        // This will happen if the password_cb function is not called,
        // which will for example be the case if the server has
        // an IP blacklist and thus always return 403.
        // In this case, there is nothing we can do.
        return false;
    }

    bool forceAuth = false;
    // If our user is forbidden to perform the
    // task we try again using the root user
    // ONLY if it was the first time
    if (status == IPP_FORBIDDEN && password_retries == 0) {
        // Pretend to be the root user
        // Sometimes setting this just works
        cupsSetUser("root");

        // force authentication
        forceAuth = true;
    }

    if (status == IPP_NOT_AUTHORIZED || status == IPP_NOT_AUTHENTICATED) {
        if (password_retries > 3 || password_retries == -1) {
            // the authentication failed 3 times
            // OR the dialog was canceled (-1)
            // reset to 0 and quit the do-while loop
            password_retries = 0;
            total_retries = 0;
            return false;
        }

        // force authentication
        forceAuth = true;
    }

    if (forceAuth) {
        // force authentication
        int ret = cupsDoAuthentication(CUPS_HTTP_DEFAULT, "POST", resource);
        qCDebug(LIBKCUPS) << "cupsDoAuthentication, return:" << ret << "password_retries:" << password_retries;

        // If the authentication was successful
        // sometimes just trying to be root works
        return ret != 0;
    }

    // the action was not forbidden
    return false;
}

const char *password_cb(const char *prompt, http_t *http, const char *method, const char *resource, void *user_data)
{
    Q_UNUSED(http)
    Q_UNUSED(method)
    Q_UNUSED(resource)

    if (++password_retries > 3) {
        // cancel the authentication
        cupsSetUser(nullptr);
        return nullptr;
    }

    auto passwordDialog = static_cast<KCupsPasswordDialog *>(user_data);
    bool wrongPassword = password_retries > 1;

    // use prompt text from CUPS callback for dialog
    passwordDialog->setPromptText(i18n("A CUPS connection requires authentication: \"%1\"", QString::fromUtf8(prompt)));

    // This will block this thread until exec is not finished
    QMetaObject::invokeMethod(passwordDialog, "exec", Qt::BlockingQueuedConnection, Q_ARG(QString, QString::fromUtf8(cupsUser())), Q_ARG(bool, wrongPassword));

    // The password dialog has just returned check the result
    // method that returns QDialog enums
    if (passwordDialog->accepted()) {
        cupsSetUser(qUtf8Printable(passwordDialog->username()));
        const auto pwd = passwordDialog->password().toUtf8();
        return pwd.constData();
    } else {
        // the dialog was canceled
        password_retries = -1;
        cupsSetUser(nullptr);
        return nullptr;
    }
}

#include "moc_KCupsConnection.cpp"
