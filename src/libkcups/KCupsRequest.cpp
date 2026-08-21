/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsRequest.h"

#include "kcupslib_log.h"

#include <KLocalizedString>

#define CUPS_DATADIR QLatin1String("/usr/share/cups")

using namespace Qt::StringLiterals;

KCupsRequest::KCupsRequest(KCupsConnection *connection)
    : m_connection(connection)
{
    // If no connection was specified use default one
    if (m_connection == nullptr) {
        m_connection = KCupsConnection::global();
    }
}

QString KCupsRequest::serverError() const
{
    switch (error()) {
    case IPP_STATUS_ERROR_SERVICE_UNAVAILABLE:
        return i18n("Print service is unavailable");
    case IPP_STATUS_ERROR_NOT_FOUND:
        return i18n("Not found");
    default: // In this case we don't want to map all enums
        qCWarning(LIBKCUPS) << "IPP status unrecognised: " << error();
        return QString::fromUtf8(ippErrorString(error()));
    }
}

void KCupsRequest::getPPDS([[maybe_unused]] const QString &make)
{
#if CUPS_VERSION_MAJOR < 3
    if (m_connection->readyToStart()) {
        KIppRequest request(CUPS_GET_PPDS, QLatin1String("/"));
        if (!make.isEmpty()) {
            request.addString(IPP_TAG_PRINTER, IPP_TAG_TEXT, KCUPS_PPD_MAKE_AND_MODEL, make);
        }

        m_ppds = m_connection->request(CUPS_HTTP_DEFAULT, request, IPP_TAG_PRINTER);

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("getPPDS", make);
    }
#endif
}

static void choose_device_cb(const char *device_class, /* I - Class */
                             const char *device_id, /* I - 1284 device ID */
                             const char *device_info, /* I - Description */
                             const char *device_make_and_model, /* I - Make and model */
                             const char *device_uri, /* I - Device URI */
                             const char *device_location, /* I - Location */
                             void *user_data) /* I - Result object */
{
    /*
     * Add the device to the array...
     */
    auto request = static_cast<KCupsRequest *>(user_data);
    QMetaObject::invokeMethod(request,
                              "device",
                              Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromUtf8(device_class)),
                              Q_ARG(QString, QString::fromUtf8(device_id)),
                              Q_ARG(QString, QString::fromUtf8(device_info)),
                              Q_ARG(QString, QString::fromUtf8(device_make_and_model)),
                              Q_ARG(QString, QString::fromUtf8(device_uri)),
                              Q_ARG(QString, QString::fromUtf8(device_location)));
}

void KCupsRequest::getDevices(int timeout)
{
    getDevices(timeout, QStringList(), QStringList());
}

void KCupsRequest::getDevices([[maybe_unused]] int timeout, [[maybe_unused]] QStringList includeSchemes, [[maybe_unused]] QStringList excludeSchemes)
{
#if CUPS_VERSION_MAJOR < 3
    if (m_connection->readyToStart()) {
        do {
            const char *include;
            if (includeSchemes.isEmpty()) {
                include = CUPS_INCLUDE_ALL;
            } else {
                include = qUtf8Printable(includeSchemes.join(QLatin1String(",")));
            }

            const char *exclude;
            if (excludeSchemes.isEmpty()) {
                exclude = CUPS_EXCLUDE_NONE;
            } else {
                exclude = qUtf8Printable(excludeSchemes.join(QLatin1String(",")));
            }

            // Scan for devices for "timeout" seconds
            cupsGetDevices(CUPS_HTTP_DEFAULT, timeout, include, exclude, (cups_device_cb_t)choose_device_cb, this);
        } while (m_connection->retry("/admin/", CUPS_GET_DEVICES));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished(true);
    } else {
        invokeMethod("getDevices", timeout, includeSchemes, excludeSchemes);
    }
#endif
}

// Deconstruct a device-uri and return http connection
// Use this for printer-direct IPP requests
static http_t *deviceConnection(const QString &uri)
{
    http_t *http = nullptr;
    char scheme[32], user[256], host[256], resource[1024];
    int port;

    if (httpSeparateURI(HTTP_URI_CODING_ALL,
                        uri.toUtf8().constData(),
                        scheme,
                        sizeof(scheme),
                        user,
                        sizeof(user),
                        host,
                        sizeof(host),
                        &port,
                        resource,
                        sizeof(resource))
        == HTTP_URI_STATUS_OK) {
        qCDebug(LIBKCUPS) << "Attempting httpConnect:" << uri;
        http = KCupsCompat::kcupsHttpConnect(host, port, nullptr, AF_UNSPEC, HTTP_ENCRYPTION_IF_REQUESTED, 1, 30000, nullptr);
    }
    return http;
}

void KCupsRequest::getPrinters(QStringList attributes, int mask)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(IPP_OP_CUPS_GET_PRINTERS, QLatin1String("/"));
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_PRINTER_TYPE, KCUPS_PRINTER_LOCAL);
        if (!attributes.isEmpty()) {
            request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_REQUESTED_ATTRIBUTES, attributes);
        }
        if (mask != -1) {
            request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_PRINTER_TYPE_MASK, mask);
        }

        const ReturnArguments ret = m_connection->request(CUPS_HTTP_DEFAULT, request, IPP_TAG_PRINTER);

        for (const QVariantMap &arguments : ret) {
            m_printers << KCupsPrinter(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinters", QVariant::fromValue(attributes), mask);
    }
}

void KCupsRequest::getAttributesDirect(const QString &printerName, const QString &uri, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        http_t *http = deviceConnection(uri);
        if (http) {
            KIppRequest request(IPP_OP_GET_PRINTER_ATTRIBUTES, QLatin1String("/"), QString(), false);
            request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, QLatin1String(KCUPS_PRINTER_URI), uri);
            request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, QLatin1String(KCUPS_REQUESTED_ATTRIBUTES), attributes);

            const auto ret = m_connection->request(http, request, IPP_TAG_PRINTER);
            for (const QVariantMap &arguments : ret) {
                QVariantMap args = arguments;
                args[KCUPS_PRINTER_NAME] = printerName;
                m_printers << KCupsPrinter(args);
            }
        } else {
            qCDebug(LIBKCUPS) << "getAttributesDirect(): unable to connect:" << uri;
        }
        setError(httpGetStatus(http), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        httpClose(http);
        setFinished();
    } else {
        invokeMethod("getAttributesDirect", printerName, uri, QVariant::fromValue(attributes));
    }
}

void KCupsRequest::getPrinterAttributes(const QString &printerName, bool isClass, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(IPP_OP_GET_PRINTER_ATTRIBUTES, QLatin1String("/"));

        request.addPrinterUri(printerName, isClass);
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, QLatin1String(KCUPS_PRINTER_TYPE), KCUPS_PRINTER_LOCAL);
        request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, QLatin1String(KCUPS_REQUESTED_ATTRIBUTES), attributes);

        const ReturnArguments ret = m_connection->request(CUPS_HTTP_DEFAULT, request, IPP_TAG_PRINTER);

        for (const QVariantMap &arguments : ret) {
            // Inject the printer name back to the arguments hash
            QVariantMap args = arguments;
            args[KCUPS_PRINTER_NAME] = printerName;
            m_printers << KCupsPrinter(args);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinterAttributes", printerName, isClass, QVariant::fromValue(attributes));
    }
}

void KCupsRequest::getJobs(const QString &printerName, bool myJobs, int whichJobs, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(IPP_OP_GET_JOBS, QLatin1String("/"));

        // printer-uri makes the Name of the Job and owner came blank lol
        request.addPrinterUri(printerName, false);
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_PRINTER_TYPE, KCUPS_PRINTER_LOCAL);
        request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_REQUESTED_ATTRIBUTES, attributes);

        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_MY_JOBS, myJobs);

        if (whichJobs == CUPS_WHICHJOBS_COMPLETED) {
            request.addString(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_WHICH_JOBS, QLatin1String("completed"));
        } else if (whichJobs == CUPS_WHICHJOBS_ALL) {
            request.addString(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_WHICH_JOBS, QLatin1String("all"));
        }

        const ReturnArguments ret = m_connection->request(CUPS_HTTP_DEFAULT, request, IPP_TAG_JOB);

        for (const QVariantMap &arguments : ret) {
            m_jobs << KCupsJob(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobs", printerName, myJobs, whichJobs, QVariant::fromValue(attributes));
    }
}

void KCupsRequest::getJobAttributes(int jobId, const QString &printerUri, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        KIppRequest request(IPP_OP_GET_JOB_ATTRIBUTES, QLatin1String("/"));

        request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_PRINTER_URI, printerUri);
        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, KCUPS_PRINTER_TYPE, KCUPS_PRINTER_LOCAL);
        request.addStringList(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, KCUPS_REQUESTED_ATTRIBUTES, attributes);

        request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

        const ReturnArguments ret = m_connection->request(CUPS_HTTP_DEFAULT, request, IPP_TAG_PRINTER);

        for (const QVariantMap &arguments : ret) {
            m_jobs << KCupsJob(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobAttributes", jobId, printerUri, QVariant::fromValue(attributes));
    }
}

#if CUPS_VERSION_MAJOR < 3
void KCupsRequest::getServerSettings()
{
    if (m_connection->readyToStart()) {
        do {
            int num_settings;
            cups_option_t *settings;
            QVariantMap arguments;
            int ret = cupsAdminGetServerSettings(CUPS_HTTP_DEFAULT, &num_settings, &settings);
            for (int i = 0; i < num_settings; ++i) {
                QString name = QString::fromUtf8(settings[i].name);
                QString value = QString::fromUtf8(settings[i].value);
                arguments[name] = value;
            }
            cupsFreeOptions(num_settings, settings);
            if (ret) {
                setError(HTTP_OK, IPP_OK, QString());
            } else {
                setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
            }

            m_server = KCupsServer(arguments);
        } while (m_connection->retry("/admin/", -1));
        setFinished();
    } else {
        invokeMethod("getServerSettings");
    }
}
#endif

void KCupsRequest::getPrinterPPD([[maybe_unused]] const QString &printerName)
{
#if CUPS_VERSION_MAJOR < 3
    if (m_connection->readyToStart()) {
        do {
            const char *filename;
            filename = cupsGetPPD2(CUPS_HTTP_DEFAULT, qUtf8Printable(printerName));
            m_ppdFile = QString::fromUtf8(filename);
        } while (m_connection->retry("/", CUPS_GET_PPD));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinterPPD", printerName);
    }
#endif
}

#if CUPS_VERSION_MAJOR < 3
void KCupsRequest::setServerSettings([[maybe_unused]] const KCupsServer &server)
{
    if (m_connection->readyToStart()) {
        do {
            QVariantMap args = server.arguments();
            int num_settings = 0;
            cups_option_t *settings = nullptr;

            QVariantMap::const_iterator i = args.constBegin();
            while (i != args.constEnd()) {
                num_settings = cupsAddOption(qUtf8Printable(i.key()), qUtf8Printable(i.value().toString()), num_settings, &settings);
                ++i;
            }

            cupsAdminSetServerSettings(CUPS_HTTP_DEFAULT, num_settings, settings);
            cupsFreeOptions(num_settings, settings);
        } while (m_connection->retry("/admin/", -1));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("setServerSettings", QVariant::fromValue(server));
    }
}
#endif

void KCupsRequest::addOrModifyPrinter(const QString &printerName, const QVariantMap &attributes, const QString &filename)
{
    KIppRequest request(IPP_OP_CUPS_ADD_MODIFY_PRINTER, QLatin1String("/admin/"), filename);
    request.addPrinterUri(printerName);
    request.addVariantValues(attributes);

    process(request);
}

void KCupsRequest::addOrModifyClass(const QString &printerName, const QVariantMap &attributes)
{
    KIppRequest request(IPP_OP_CUPS_ADD_MODIFY_CLASS, QLatin1String("/admin/"));
    request.addPrinterUri(printerName, true);
    request.addVariantValues(attributes);

    process(request);
}

void KCupsRequest::setShared(const QString &printerName, bool isClass, bool shared)
{
    KIppRequest request(isClass ? IPP_OP_CUPS_ADD_MODIFY_CLASS : IPP_OP_CUPS_ADD_MODIFY_PRINTER, QLatin1String("/admin/"));
    request.addPrinterUri(printerName, isClass);
    request.addBoolean(IPP_TAG_OPERATION, KCUPS_PRINTER_IS_SHARED, shared);

    process(request);
}

void KCupsRequest::pausePrinter(const QString &printerName)
{
    KIppRequest request(IPP_OP_PAUSE_PRINTER, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::resumePrinter(const QString &printerName)
{
    KIppRequest request(IPP_OP_RESUME_PRINTER, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::rejectJobs(const QString &printerName)
{
    KIppRequest request(IPP_OP_CUPS_REJECT_JOBS, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::acceptJobs(const QString &printerName)
{
    KIppRequest request(IPP_OP_CUPS_ACCEPT_JOBS, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::setDefaultPrinter(const QString &printerName)
{
    KIppRequest request(IPP_OP_CUPS_SET_DEFAULT, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::deletePrinter(const QString &printerName)
{
    KIppRequest request(IPP_OP_CUPS_DELETE_PRINTER, QLatin1String("/admin/"));
    request.addPrinterUri(printerName);

    process(request);
}

void KCupsRequest::printTestPage(const QString &printerName, bool isClass)
{
    QString resource; /* POST resource path */
    QString filename; /* Test page filename */
    QString datadir; /* CUPS_DATADIR env var */

    /*
     * Locate the test page file...
     */
    datadir = QString::fromUtf8(qgetenv("CUPS_DATADIR"));
    if (datadir.isEmpty()) {
        datadir = CUPS_DATADIR;
    }
    filename = datadir % QLatin1String("/data/testprint");

    /*
     * Point to the printer/class...
     */
    if (isClass) {
        resource = QLatin1String("/classes/") + printerName;
    } else {
        resource = QLatin1String("/printers/") + printerName;
    }

    KIppRequest request(IPP_OP_PRINT_JOB, resource, filename);
    request.addPrinterUri(printerName);
    request.addString(IPP_TAG_OPERATION, IPP_TAG_NAME, KCUPS_JOB_NAME, i18n("Test Page"));

    process(request);
}

void KCupsRequest::printCommand([[maybe_unused]] const QString &printerName, [[maybe_unused]] const QString &command, [[maybe_unused]] const QString &title)
{
#if CUPS_VERSION_MAJOR < 3
    if (m_connection->readyToStart()) {
        do {
            int job_id; /* Command file job */
            char command_file[1024]; /* Command "file" */
            http_status_t status; /* Document status */
            cups_option_t hold_option; /* job-hold-until option */

            /*
             * Create the CUPS command file...
             */
            snprintf(command_file, sizeof(command_file), "#CUPS-COMMAND\n%s\n", command.toUtf8().constData());

            /*
             * Send the command file job...
             */
            hold_option.name = const_cast<char *>("job-hold-until");
            hold_option.value = const_cast<char *>("no-hold");

            if ((job_id = cupsCreateJob(CUPS_HTTP_DEFAULT, qUtf8Printable(printerName), qUtf8Printable(title), 1, &hold_option)) < 1) {
                qCWarning(LIBKCUPS) << "Unable to send command to printer driver!";

                setError(HTTP_OK, IPP_STATUS_ERROR_NOT_POSSIBLE, i18n("Unable to send command to printer driver!"));
                setFinished();
                return;
            }

            status = cupsStartDocument(CUPS_HTTP_DEFAULT, qUtf8Printable(printerName), job_id, nullptr, CUPS_FORMAT_COMMAND, 1);
            if (status == HTTP_STATUS_CONTINUE) {
                status = cupsWriteRequestData(CUPS_HTTP_DEFAULT, command_file, strlen(command_file));
            }

            if (status == HTTP_STATUS_CONTINUE) {
                cupsFinishDocument(CUPS_HTTP_DEFAULT, qUtf8Printable(printerName));
            }

            setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
            if (httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError() >= IPP_STATUS_REDIRECTION_OTHER_SITE) {
                qCWarning(LIBKCUPS) << "Unable to send command to printer driver!";

                cupsCancelJob(qUtf8Printable(printerName), job_id);
                setFinished();
                return; // Return to avoid a new try
            }
        } while (m_connection->retry("/", IPP_OP_CREATE_JOB));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("printCommand", printerName, command, title);
    }
#endif
}

void KCupsRequest::cancelJob(const QString &printerName, int jobId)
{
    KIppRequest request(IPP_OP_CANCEL_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    process(request);
}

void KCupsRequest::holdJob(const QString &printerName, int jobId)
{
    KIppRequest request(IPP_OP_HOLD_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    process(request);
}

void KCupsRequest::releaseJob(const QString &printerName, int jobId)
{
    KIppRequest request(IPP_OP_RELEASE_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    process(request);
}

void KCupsRequest::restartJob(const QString &printerName, int jobId)
{
    KIppRequest request(IPP_OP_RESTART_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    process(request);
}

void KCupsRequest::moveJob(const QString &fromPrinterName, int jobId, const QString &toPrinterName)
{
    if (jobId < -1 || fromPrinterName.isEmpty() || toPrinterName.isEmpty() || jobId == 0) {
        qCWarning(LIBKCUPS) << "Internal error, invalid input data" << jobId << fromPrinterName << toPrinterName;
        setFinished();
        return;
    }

    KIppRequest request(IPP_OP_CUPS_MOVE_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(fromPrinterName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);

    QString toPrinterUri = KIppRequest::assembleUrif(toPrinterName, false);
    request.addString(IPP_TAG_OPERATION, IPP_TAG_URI, KCUPS_JOB_PRINTER_URI, toPrinterUri);

    process(request);
}

void KCupsRequest::authenticateJob(const QString &printerName, const QStringList authInfo, int jobId)
{
    KIppRequest request(IPP_OP_CUPS_AUTHENTICATE_JOB, QLatin1String("/jobs/"));
    request.addPrinterUri(printerName);
    request.addInteger(IPP_TAG_OPERATION, IPP_TAG_INTEGER, KCUPS_JOB_ID, jobId);
    request.addStringList(IPP_TAG_OPERATION, IPP_TAG_TEXT, KCUPS_AUTH_INFO, authInfo);

    process(request);
}

void KCupsRequest::invokeMethod(const char *method,
                                const QVariant &arg1,
                                const QVariant &arg2,
                                const QVariant &arg3,
                                const QVariant &arg4,
                                const QVariant &arg5,
                                const QVariant &arg6,
                                const QVariant &arg7,
                                const QVariant &arg8)
{
    m_error = IPP_STATUS_OK;
    m_errorMsg.clear();
    m_printers.clear();
    m_jobs.clear();
    m_ppds.clear();
    m_ppdFile.clear();

    // If this fails we get into a infinite loop
    // Do not use global()->thread() which point
    // to the KCupsConnection parent thread
    moveToThread(m_connection);

    m_finished = !QMetaObject::invokeMethod(this,
                                            method,
                                            Qt::QueuedConnection,
                                            QGenericArgument(arg1.typeName(), arg1.data()),
                                            QGenericArgument(arg2.typeName(), arg2.data()),
                                            QGenericArgument(arg3.typeName(), arg3.data()),
                                            QGenericArgument(arg4.typeName(), arg4.data()),
                                            QGenericArgument(arg5.typeName(), arg5.data()),
                                            QGenericArgument(arg6.typeName(), arg6.data()),
                                            QGenericArgument(arg7.typeName(), arg7.data()),
                                            QGenericArgument(arg8.typeName(), arg8.data()));
    if (m_finished) {
        setError(HTTP_STATUS_ERROR, IPP_STATUS_ERROR_BAD_REQUEST, i18n("Failed to invoke method: %1", QLatin1String(method)));
        setFinished();
    }
}

void KCupsRequest::process(const KIppRequest &request)
{
    if (m_connection->readyToStart()) {
        m_connection->request(CUPS_HTTP_DEFAULT, request);

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished();
    } else {
        invokeMethod("process", QVariant::fromValue(request));
    }
}

ReturnArguments KCupsRequest::ppds() const
{
    return m_ppds;
}

#if CUPS_VERSION_MAJOR < 3
KCupsServer KCupsRequest::serverSettings() const
{
    return m_server;
}
#endif

QString KCupsRequest::printerPPD() const
{
    return m_ppdFile;
}

KCupsPrinters KCupsRequest::printers() const
{
    return m_printers;
}

KCupsJobs KCupsRequest::jobs() const
{
    return m_jobs;
}

void KCupsRequest::waitTillFinished()
{
    if (m_finished) {
        return;
    }

    if (!m_loopConnect) {
        m_loopConnect = connect(this, &KCupsRequest::finished, &m_loop, &QEventLoop::quit);
    }

    m_loop.exec();
}

bool KCupsRequest::hasError() const
{
    return m_error;
}

ipp_status_t KCupsRequest::error() const
{
    return m_error;
}

http_status_t KCupsRequest::httpStatus() const
{
    return m_httpStatus;
}

QString KCupsRequest::errorMsg() const
{
    return m_errorMsg;
}

KCupsConnection *KCupsRequest::connection() const
{
    return m_connection;
}

void KCupsRequest::setError(http_status_t httpStatus, ipp_status_t error, const QString &errorMsg)
{
    m_httpStatus = httpStatus;
    m_error = error;
    m_errorMsg = errorMsg;
}

void KCupsRequest::setFinished(bool delayed)
{
    m_finished = true;
    if (delayed) {
        QTimer::singleShot(0, this, [this]() {
            Q_EMIT finished(this);
        });
    } else {
        Q_EMIT finished(this);
    }
}

// callback for cupsEnumDests
static int get_dest_cb(void *user_data, [[maybe_unused]] unsigned flags, cups_dest_t *dest)
{
    static const QStringList s_attrs({KCUPS_PRINTER_STATE,
                                      KCUPS_PRINTER_STATE_MESSAGE,
                                      KCUPS_PRINTER_IS_SHARED,
                                      KCUPS_PRINTER_IS_ACCEPTING_JOBS,
                                      KCUPS_PRINTER_LOCATION,
                                      KCUPS_PRINTER_MAKE_AND_MODEL,
                                      KCUPS_PRINTER_COMMANDS,
                                      KCUPS_MARKER_CHANGE_TIME,
                                      KCUPS_MARKER_COLORS,
                                      KCUPS_MARKER_LEVELS,
                                      KCUPS_MARKER_NAMES,
                                      KCUPS_MARKER_TYPES,
                                      KCUPS_AUTH_INFO_REQUIRED});

    // get the identifying options
    auto uriSupported = QString::fromUtf8(cupsGetOption("printer-uri-supported", dest->num_options, dest->options));
    auto devUri = QString::fromUtf8(cupsGetOption("device-uri", dest->num_options, dest->options));
    auto pInfo = QString::fromUtf8(cupsGetOption("printer-info", dest->num_options, dest->options));
    auto type = std::atoi(cupsGetOption("printer-type", dest->num_options, dest->options));

    // determine printer name
    QString pname;
    if ((type & KCUPS_PRINTER_DISCOVERED) | uriSupported.isEmpty()) {
        if (devUri.isEmpty()) {
            qCDebug(LIBKCUPS) << "Unknown Device:" << uriSupported << type << pInfo;
            return 1;
        }
        // uriSupported is null when discovered
        pname = QString(u"_discovered_%1"_s).arg(pInfo.replace(u"/ /g"_s, u"_"_s));
        qCDebug(LIBKCUPS) << "Discovered Device:" << pname << type << pInfo;
    } else {
        // configured queue, name is the end string of the uri
        const auto l = uriSupported.split(QLatin1String("/"));
        if (l.count() > 0) {
            pname = l[l.count() - 1];
            qCDebug(LIBKCUPS) << "Configured printer:" << uriSupported << type << pInfo;
        } else {
            qCDebug(LIBKCUPS) << "Unable to determine printer name:" << uriSupported;
            return 1;
        }
    }

    // Build the printer
    KCupsPrinter printer({{KCUPS_PRINTER_NAME, pname},
                          {KCUPS_PRINTER_TYPE, type},
                          {KCUPS_PRINTER_INFO, pInfo},
                          {KCUPS_DEVICE_URI, devUri},
                          {KCUPS_PRINTER_URI_SUPPORTED, uriSupported}});
    for (const auto &k : s_attrs) {
        printer.setAttribute(k, QString::fromUtf8(cupsGetOption(k.toUtf8().data(), dest->num_options, dest->options)));
    }

    // Markers: options comma separated strings, for the lists ("3","55")
    if (auto m = printer.argument(KCUPS_MARKER_NAMES).toString(); !m.isEmpty()) {
        qCDebug(LIBKCUPS) << "Setting Markers attributes" << pname;

        const auto toList = [](const QString &option) -> QStringList {
            return option.split(QLatin1String(","));
        };

        const auto toIntList = [](const QStringList &list) -> QList<int> {
            QList<int> intList;

            for (const QString &str : list) {
                bool ok;
                int value = str.toInt(&ok);
                if (ok) {
                    intList.append(value);
                }
            }
            return intList;
        };

        // convert strings to lists
        printer.setAttribute(KCUPS_MARKER_NAMES, toList(m.replace(QLatin1String("\\"), QLatin1String(""))));
        printer.setAttribute(KCUPS_MARKER_LEVELS, QVariant::fromValue(toIntList(toList(printer.argument(KCUPS_MARKER_LEVELS).toString()))));
        printer.setAttribute(KCUPS_MARKER_COLORS, toList(printer.argument(KCUPS_MARKER_COLORS).toString()));
        printer.setAttribute(KCUPS_MARKER_TYPES, toList(printer.argument(KCUPS_MARKER_TYPES).toString()));
    }

    // if Class, get member names
    if (printer.isClass()) {
        // since this is a static cb, we have to roll a "raw" request
        ipp_t *request = ippNewRequest(IPP_OP_GET_PRINTER_ATTRIBUTES);

        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uriSupported.toUtf8().data());
        ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "requested-attributes", NULL, "member-names");

        ipp_t *response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");
        ipp_attribute_t *members = ippFindAttribute(response, "member-names", IPP_TAG_NAME);

        int i, count = ippGetCount(members);
        QStringList memberList;
        for (i = 0; i < count; i++) {
            memberList << QString::fromUtf8(ippGetString(members, i, NULL));
        }
        printer.setAttribute(KCUPS_MEMBER_NAMES, memberList);
        ippDelete(response);
    }

    // "emit" signal with the printer
    QMetaObject::invokeMethod(static_cast<KCupsRequest *>(user_data), "destination", Qt::QueuedConnection, Q_ARG(KCupsPrinter, printer));

    return (1);
}

void KCupsRequest::getDestinations(int timeout, uint type, uint mask)
{
    if (m_connection->readyToStart()) {
        cupsEnumDests(CUPS_DEST_FLAGS_NONE, timeout, NULL, type, mask, (cups_dest_cb_t)get_dest_cb, this);
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), KCupsCompat::kcupsError(), QString::fromUtf8(KCupsCompat::kcupsErrorString()));
        setFinished(true);
    } else {
        invokeMethod("getDestinations", timeout, type, mask);
    }
}

#include "moc_KCupsRequest.cpp"
