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

#include "KCupsRequest.h"

#include "KCupsJob.h"
#include "KCupsPrinter.h"

#include <KLocale>
#include <KDebug>
#include <QStringBuilder>

#include <cups/adminutil.h>

#define CUPS_DATADIR    "/usr/share/cups"

KCupsRequest::KCupsRequest(KCupsConnection *connection) :
    m_connection(connection),
    m_finished(true),
    m_error(IPP_OK)
{
    // If no connection was specified use default one
    if (m_connection == 0) {
        m_connection = KCupsConnection::global();
    }
    connect(this, SIGNAL(finished()), &m_loop, SLOT(quit()));
}

QString KCupsRequest::serverError() const
{
    switch (error()) {
    case IPP_SERVICE_UNAVAILABLE:
        return i18n("Service is unavailable");
    case IPP_NOT_FOUND :
        return i18n("Not found");
    default : // In this case we don't want to map all enums
        kWarning() << "status unrecognised: " << error();
        return QString();
    }
}

void KCupsRequest::getPPDS(const QString &make)
{
    if (m_connection->readyToStart()) {
        QVariantHash request;
        if (!make.isEmpty()){
            request["ppd-make-and-model"] = make;
        }
        request["need-dest-name"] = false;

        m_ppds = m_connection->request(CUPS_GET_PPDS,
                                       "/",
                                       request,
                                       true);
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPPDS", make);
    }
}

static void choose_device_cb(const char *device_class,           /* I - Class */
                             const char *device_id,              /* I - 1284 device ID */
                             const char *device_info,            /* I - Description */
                             const char *device_make_and_model,  /* I - Make and model */
                             const char *device_uri,             /* I - Device URI */
                             const char *device_location,        /* I - Location */
                             void *user_data)                    /* I - Result object */
{
    /*
     * Add the device to the array...
     */
    KCupsRequest *request = static_cast<KCupsRequest*>(user_data);
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

void KCupsRequest::getDevices(int timeout, QStringList includeSchemes, QStringList excludeSchemes)
{
    if (m_connection->readyToStart()) {
        do {
            const char *include;
            if (includeSchemes.isEmpty()) {
                include = CUPS_INCLUDE_ALL;
            } else {
                include = includeSchemes.join(QLatin1String(",")).toUtf8();
            }

            const char *exclude;
            if (excludeSchemes.isEmpty()) {
                exclude = CUPS_EXCLUDE_NONE;
            } else {
                exclude = excludeSchemes.join(QLatin1String(",")).toUtf8();
            }

            // Scan for devices for "timeout" seconds
            cupsGetDevices(CUPS_HTTP_DEFAULT,
                           timeout,
                           include,
                           exclude,
                           (cups_device_cb_t) choose_device_cb,
                           this);
        } while (m_connection->retry("/admin/"));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished(true);
    } else {
        invokeMethod("getDevices", timeout, includeSchemes, excludeSchemes);
    }
}

// THIS function can get the default server dest through the
// "printer-is-default" attribute BUT it does not get user
// defined default printer, see cupsGetDefault() on www.cups.org for details

void KCupsRequest::getPrinters(QStringList attributes, cups_ptype_t mask)
{
    QVariantHash arguments;
    arguments[KCUPS_PRINTER_TYPE_MASK] = mask;
    getPrinters(attributes, arguments);
}

void KCupsRequest::getPrinters(QStringList attributes, const QVariantHash &arguments)
{
    if (m_connection->readyToStart()) {
        QVariantHash request = arguments;
        request[KCUPS_PRINTER_TYPE] = CUPS_PRINTER_LOCAL;
        request["requested-attributes"] = attributes;
        request["need-dest-name"] = true;

        ReturnArguments ret;
        ret = m_connection->request(CUPS_GET_PRINTERS,
                                    "/",
                                    request,
                                    true);

        foreach (const QVariantHash &arguments, ret) {
            m_printers << KCupsPrinter(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinters", qVariantFromValue(attributes), arguments);
    }
}

void KCupsRequest::getPrinterAttributes(const QString &printerName, bool isClass, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        QVariantHash request;
        request[KCUPS_PRINTER_NAME] = printerName;
        request["printer-is-class"] = isClass;
        request["need-dest-name"] = false; // we don't need a dest name since it's a single item
        request["requested-attributes"] = attributes;

        ReturnArguments ret;
        ret = m_connection->request(IPP_GET_PRINTER_ATTRIBUTES,
                                    "/admin/",
                                    request,
                                    true);

        foreach (const QVariantHash &arguments, ret) {
            // Inject the printer name back to the arguments hash
            QVariantHash args = arguments;
            args[KCUPS_PRINTER_NAME] = printerName;
            m_printers << KCupsPrinter(args);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinterAttributes", printerName, isClass, qVariantFromValue(attributes));
    }
}

void KCupsRequest::getJobs(const QString &printerName, bool myJobs, int whichJobs, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        QVariantHash request;
        // printer-uri makes the Name of the Job and owner came blank lol
        request[KCUPS_PRINTER_NAME] = printerName;

        if (myJobs) {
            request["my-jobs"] = myJobs;
        }

        if (whichJobs == CUPS_WHICHJOBS_COMPLETED) {
            request["which-jobs"] = "completed";
        } else if (whichJobs == CUPS_WHICHJOBS_ALL) {
            request["which-jobs"] = "all";
        }

        if (!attributes.isEmpty()) {
            request["requested-attributes"] = attributes;
        }
        request["group-tag-qt"] = IPP_TAG_JOB;

        ReturnArguments ret;
        ret = m_connection->request(IPP_GET_JOBS,
                                    "/",
                                    request,
                                    true);
        foreach (const QVariantHash &arguments, ret) {
            m_jobs << KCupsJob(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobs", printerName, myJobs, whichJobs, qVariantFromValue(attributes));
    }
}

void KCupsRequest::getJobAttributes(int jobId, const QString &printerUri, QStringList attributes)
{
    if (m_connection->readyToStart()) {
        QVariantHash request;
        request[KCUPS_JOB_ID] = jobId;
        request[KCUPS_PRINTER_URI] = printerUri;
        request["need-dest-name"] = false; // we don't need a dest name since it's a single list
        request["requested-attributes"] = attributes;

        ReturnArguments ret;
        ret = m_connection->request(IPP_GET_JOB_ATTRIBUTES,
                                    "/admin/",
                                    request,
                                    true);

        foreach (const QVariantHash &arguments, ret) {
            m_jobs << KCupsJob(arguments);
        }

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobAttributes", jobId, printerUri, qVariantFromValue(attributes));
    }
}

void KCupsRequest::getServerSettings()
{
    if (m_connection->readyToStart()) {
        do {
            int num_settings;
            cups_option_t *settings;
            QVariantHash arguments;
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
                setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
            }

            m_server = KCupsServer(arguments);
        } while (m_connection->retry("/admin/"));
        setFinished();
    } else {
        invokeMethod("getServerSettings");
    }
}

void KCupsRequest::getPrinterPPD(const QString &printerName)
{
    if (m_connection->readyToStart()) {
        do {
            const char  *filename;
            filename = cupsGetPPD2(CUPS_HTTP_DEFAULT, printerName.toUtf8());
            kDebug() << filename;
            m_ppdFile = filename;
            kDebug() << m_ppdFile;
        } while (m_connection->retry("/"));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinterPPD", printerName);
    }
}

void KCupsRequest::setServerSettings(const KCupsServer &server)
{
    if (m_connection->readyToStart()) {
        do {
            QVariantHash args = server.arguments();
            int num_settings = 0;
            cups_option_t *settings;

            QVariantHash::const_iterator i = args.constBegin();
            while (i != args.constEnd()) {
                num_settings = cupsAddOption(i.key().toUtf8(),
                                             i.value().toString().toUtf8(),
                                             num_settings,
                                             &settings);
                ++i;
            }

            cupsAdminSetServerSettings(CUPS_HTTP_DEFAULT, num_settings, settings);
            cupsFreeOptions(num_settings, settings);
        } while (m_connection->retry("/admin/"));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setServerSettings", qVariantFromValue(server));
    }
}

void KCupsRequest::addOrModifyPrinter(const QString &printerName, const QVariantHash &attributes, const QString &filename)
{
    QVariantHash request = attributes;
    request[KCUPS_PRINTER_NAME] = printerName;
    request["printer-is-class"] = false;
    if (!filename.isEmpty()) {
        request["filename"] = filename;
    }

    doOperation(CUPS_ADD_MODIFY_PRINTER, QLatin1String("/admin/"), request);
}

void KCupsRequest::addOrModifyClass(const QString &printerName, const QVariantHash &attributes)
{
    QVariantHash request = attributes;
    request[KCUPS_PRINTER_NAME] = printerName;
    request["printer-is-class"] = true;

    doOperation(CUPS_ADD_MODIFY_CLASS, QLatin1String("/admin/"), request);
}

void KCupsRequest::setShared(const QString &printerName, bool isClass, bool shared)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    request["printer-is-class"] = isClass;
    request[KCUPS_PRINTER_IS_SHARED] = shared;
    request["need-dest-name"] = true;

    ipp_op_e operation;
    operation = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;

    doOperation(operation, QLatin1String("/admin/"), request);
}

void KCupsRequest::pausePrinter(const QString &printerName)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    doOperation(IPP_PAUSE_PRINTER, QLatin1String("/admin/"), request);
}

void KCupsRequest::resumePrinter(const QString &printerName)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    doOperation(IPP_RESUME_PRINTER, QLatin1String("/admin/"), request);
}

void KCupsRequest::rejectJobs(const QString &printer)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printer;
    doOperation(CUPS_REJECT_JOBS, QLatin1String("/admin/"), request);
}

void KCupsRequest::acceptJobs(const QString &printerName)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    doOperation(CUPS_ACCEPT_JOBS, QLatin1String("/admin/"), request);
}

void KCupsRequest::setDefaultPrinter(const QString &printerName)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    doOperation(CUPS_SET_DEFAULT, QLatin1String("/admin/"), request);
}

void KCupsRequest::deletePrinter(const QString &printerName)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    doOperation(CUPS_DELETE_PRINTER, QLatin1String("/admin/"), request);
}

void KCupsRequest::printTestPage(const QString &printerName, bool isClass)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    request["printer-is-class"] = isClass;
    request[KCUPS_JOB_NAME] = i18n("Test Page");
    QString resource; /* POST resource path */
    QString filename; /* Test page filename */
    QString datadir;  /* CUPS_DATADIR env var */

    /*
     * Locate the test page file...
     */
    datadir = qgetenv("CUPS_DATADIR");
    if (datadir.isEmpty()) {
        datadir = CUPS_DATADIR;
    }
    filename = datadir % QLatin1String("/data/testprint");
    request["filename"] = filename;

    /*
     * Point to the printer/class...
     */
    if (isClass) {
        resource = QLatin1String("/classes/") % printerName;
    } else {
        resource = QLatin1String("/printers/") % printerName;
    }

    doOperation(IPP_PRINT_JOB, resource, request);
}

void KCupsRequest::printCommand(const QString &printerName, const QString &command, const QString &title)
{
    if (m_connection->readyToStart()) {
        do {
            int           job_id;                 /* Command file job */
            char          command_file[1024];     /* Command "file" */
            http_status_t status;                 /* Document status */
            cups_option_t hold_option;            /* job-hold-until option */

            /*
             * Create the CUPS command file...
             */
            snprintf(command_file, sizeof(command_file), "#CUPS-COMMAND\n%s\n", command.toUtf8().data());

            /*
             * Send the command file job...
             */
            hold_option.name  = const_cast<char*>("job-hold-until");
            hold_option.value = const_cast<char*>("no-hold");

            if ((job_id = cupsCreateJob(CUPS_HTTP_DEFAULT,
                                        printerName.toUtf8(),
                                        title.toUtf8(),
                                        1,
                                        &hold_option)) < 1) {
                qWarning() << "Unable to send command to printer driver!";

                setError(HTTP_OK, IPP_NOT_POSSIBLE, i18n("Unable to send command to printer driver!"));
                setFinished();
                return;
            }

            status = cupsStartDocument(CUPS_HTTP_DEFAULT,
                                       printerName.toUtf8(),
                                       job_id,
                                       NULL,
                                       CUPS_FORMAT_COMMAND,
                                       1);
            if (status == HTTP_CONTINUE) {
                status = cupsWriteRequestData(CUPS_HTTP_DEFAULT, command_file,
                                              strlen(command_file));
            }

            if (status == HTTP_CONTINUE) {
                cupsFinishDocument(CUPS_HTTP_DEFAULT, printerName.toUtf8());
            }

            setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
            if (httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError() >= IPP_REDIRECTION_OTHER_SITE) {
                qWarning() << "Unable to send command to printer driver!";

                cupsCancelJob(printerName.toUtf8(), job_id);
                setFinished();
                return; // Return to avoid a new try
            }
        } while (m_connection->retry("/"));
        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("printCommand", printerName, command, title);
    }
}

void KCupsRequest::cancelJob(const QString &printerName, int jobId)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    request[KCUPS_JOB_ID] = jobId;

    doOperation(IPP_CANCEL_JOB, QLatin1String("/jobs/"), request);
}

void KCupsRequest::holdJob(const QString &printerName, int jobId)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    request[KCUPS_JOB_ID] = jobId;

    doOperation(IPP_HOLD_JOB, QLatin1String("/jobs/"), request);
}

void KCupsRequest::releaseJob(const QString &printerName, int jobId)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    request[KCUPS_JOB_ID] = jobId;

    doOperation(IPP_RELEASE_JOB, QLatin1String("/jobs/"), request);
}

void KCupsRequest::restartJob(const QString &printerName, int jobId)
{
    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = printerName;
    request[KCUPS_JOB_ID] = jobId;

    doOperation(IPP_RESTART_JOB, QLatin1String("/jobs/"), request);
}

void KCupsRequest::moveJob(const QString &fromPrinterName, int jobId, const QString &toPrinterName)
{
    if (jobId < -1 || fromPrinterName.isEmpty() || toPrinterName.isEmpty() || jobId == 0) {
        qWarning() << "Internal error, invalid input data" << jobId << fromPrinterName << toPrinterName;
        setFinished();
        return;
    }

    QVariantHash request;
    request[KCUPS_PRINTER_NAME] = fromPrinterName;
    request[KCUPS_JOB_ID] = jobId;
    request[KCUPS_JOB_PRINTER_URI] = toPrinterName;

    doOperation(CUPS_MOVE_JOB, QLatin1String("/jobs/"), request);
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
    m_error = IPP_OK;
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
        setError(HTTP_ERROR, IPP_BAD_REQUEST, i18n("Failed to invoke method: %1", method));
        setFinished();
    }
}

void KCupsRequest::doOperation(int operation, const QString &resource, const QVariantHash &request)
{
    if (m_connection->readyToStart()) {
        m_connection->request(static_cast<ipp_op_e>(operation),
                              resource.toUtf8(),
                              request,
                              false);

        setError(httpGetStatus(CUPS_HTTP_DEFAULT), cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("doOperation", operation, resource, request);
    }
}

ReturnArguments KCupsRequest::ppds() const
{
    return m_ppds;
}

KCupsServer KCupsRequest::serverSettings() const
{
    return m_server;
}

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
        QTimer::singleShot(0, this, SIGNAL(finished()));
    } else {
        emit finished();
    }
}
