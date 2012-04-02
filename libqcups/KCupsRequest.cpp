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
#include "KCupsServer.h"

#include <KLocale>
#include <KDebug>
#include <QStringBuilder>

#include <cups/adminutil.h>

#define CUPS_DATADIR    "/usr/share/cups"

KCupsRequest::KCupsRequest() :
    m_finished(true),
    m_error(false)
{
    connect(this, SIGNAL(finished()), &m_loop, SLOT(quit()));
    qRegisterMetaType<KCupsJob>("KCupsJob");
    qRegisterMetaType<KCupsJob::Attributes>("KCupsJob::Attributes");
    qRegisterMetaType<KCupsPrinter>("KCupsPrinter");
    qRegisterMetaType<KCupsPrinter::Attributes>("KCupsPrinter::Attributes");
    qRegisterMetaType<KCupsServer>("KCupsServer");
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
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        if (!make.isEmpty()){
            request["ppd-make-and-model"] = make;
        }
        request["need-dest-name"] = false;

        m_retArguments = KCupsConnection::request(CUPS_GET_PPDS,
                                                  "/",
                                                  request,
                                                  true);
        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
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

void KCupsRequest::getDevices()
{
    if (KCupsConnection::readyToStart()) {
        do {
            // Scan for devices for 30 seconds
            // TODO change back to 30
            cupsGetDevices(CUPS_HTTP_DEFAULT,
                           5,
                           CUPS_INCLUDE_ALL,
                           CUPS_EXCLUDE_NONE,
                           (cups_device_cb_t) choose_device_cb,
                           this);
        } while (KCupsConnection::retryIfForbidden());
        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getDevices");
    }
}

// THIS function can get the default server dest through the
// "printer-is-default" attribute BUT it does not get user
// defined default printer, see cupsGetDefault() on www.cups.org for details

void KCupsRequest::getPrinters(KCupsPrinter::Attributes attributes, cups_ptype_t mask)
{
    QVariantHash arguments;
    arguments["printer-type-mask"] = mask;
    getPrinters(attributes, arguments);
}

void KCupsRequest::getPrinters(KCupsPrinter::Attributes attributes, const QVariantHash &arguments)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request = arguments;
        request["printer-type"] = CUPS_PRINTER_LOCAL;
        request["requested-attributes"] = KCupsPrinter::flags(attributes);
        request["need-dest-name"] = true;

        ReturnArguments ret;
        ret = KCupsConnection::request(CUPS_GET_PRINTERS,
                                       QLatin1String("/"),
                                       request,
                                       true);

        for (int i = 0; i < ret.size(); i++) {
            m_printers << KCupsPrinter(ret.at(i));
        }

        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinters", qVariantFromValue(attributes), arguments);
    }
}

void KCupsRequest::getPrinterAttributes(const QString &printer, bool isClass, KCupsPrinter::Attributes attributes)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        request["printer-name"] = printer;
        request["printer-is-class"] = isClass;
        request["need-dest-name"] = false; // we don't need a dest name since it's a single list
        request["requested-attributes"] = KCupsPrinter::flags(attributes);

        ReturnArguments ret;
        ret = KCupsConnection::request(IPP_GET_PRINTER_ATTRIBUTES,
                                       QLatin1String("/admin/"),
                                       request,
                                       true);

        for (int i = 0; i < ret.size(); i++) {
            QVariantHash arguments = ret.at(i);
            // Inject the printer name back to the arguments hash
            arguments["printer-name"] = printer;
            m_printers << KCupsPrinter(arguments);
        }

        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinterAttributes", printer, isClass, qVariantFromValue(attributes));
    }
}

void KCupsRequest::getJobs(const QString &printer, bool myJobs, int whichJobs, KCupsJob::Attributes attributes)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        // This makes the Name of the Job and owner came blank lol
//        if (printer.isEmpty()) {
//            request["printer-uri"] = printer;
//        } else {
            request["printer-name"] = printer;
//        }

        if (myJobs) {
            request["my-jobs"] = myJobs;
        }

        if (whichJobs == CUPS_WHICHJOBS_COMPLETED) {
            request["which-jobs"] = "completed";
        } else if (whichJobs == CUPS_WHICHJOBS_ALL) {
            request["which-jobs"] = "all";
        }

        QStringList attributesStrList = KCupsJob::flags(attributes);
        if (!attributesStrList.isEmpty()) {
            request["requested-attributes"] = attributesStrList;
        }
        request["group-tag-qt"] = IPP_TAG_JOB;

        ReturnArguments ret;
        ret = KCupsConnection::request(IPP_GET_JOBS,
                                       QLatin1String("/"),
                                       request,
                                       true);

        for (int i = 0; i < ret.size(); i++) {
            m_jobs << KCupsJob(ret.at(i));
        }

        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobs", printer, myJobs, whichJobs, qVariantFromValue(attributes));
    }
}

void KCupsRequest::addClass(const QVariantHash &values)
{
    QVariantHash request = values;
    request["printer-is-class"] = true;
    request["printer-is-accepting-jobs"] = true;
    request["printer-state"] = IPP_PRINTER_IDLE;

    doOperation(CUPS_ADD_MODIFY_CLASS, QLatin1String("/admin/"), request);
}

void KCupsRequest::getServerSettings()
{
    if (KCupsConnection::readyToStart()) {
        do {
            int num_settings;
            cups_option_t *settings;
            QVariantHash arguments;
            cupsAdminGetServerSettings(CUPS_HTTP_DEFAULT, &num_settings, &settings);
            for (int i = 0; i < num_settings; ++i) {
                QString name = QString::fromUtf8(settings[i].name);
                QString value = QString::fromUtf8(settings[i].value);
                arguments[name] = value;
            }
            cupsFreeOptions(num_settings, settings);

            emit server(KCupsServer(arguments));
        } while (KCupsConnection::retryIfForbidden());
        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getServerSettings");
    }
}

void KCupsRequest::setServerSettings(const KCupsServer &server)
{
    if (KCupsConnection::readyToStart()) {
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
        } while (KCupsConnection::retryIfForbidden());
        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setServerSettings", qVariantFromValue(server));
    }
}

void KCupsRequest::setAttributes(const QString &printer,
                                 bool isClass,
                                 const QVariantHash &attributes,
                                 const QString &filename)
{
    if (attributes.isEmpty()) {
        setFinished();
        return;
    }

    QVariantHash request = attributes;
    request["printer-name"] = printer;
    request["printer-is-class"] = isClass;
    if (!filename.isEmpty()) {
        request["filename"] = filename;
    }

    ipp_op_e operation;
    // TODO this seems weird now.. review this code..
    if (isClass && request.contains("member-uris")) {
        operation = CUPS_ADD_MODIFY_CLASS;
    } else {
        operation = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;
    }

    doOperation(operation, QLatin1String("/admin/"), request);
}

void KCupsRequest::setShared(const QString &printer, bool isClass, bool shared)
{
    QVariantHash request;
    request["printer-name"] = printer;
    request["printer-is-class"] = isClass;
    request["printer-is-shared"] = shared;
    request["need-dest-name"] = true;

    ipp_op_e operation;
    operation = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;

    doOperation(operation, QLatin1String("/admin/"), request);
}

void KCupsRequest::pausePrinter(const QString &name)
{
    QVariantHash request;
    request["printer-name"] = name;
    doOperation(IPP_PAUSE_PRINTER, QLatin1String("/admin/"), request);
}

void KCupsRequest::resumePrinter(const QString &name)
{
    QVariantHash request;
    request["printer-name"] = name;
    doOperation(IPP_RESUME_PRINTER, QLatin1String("/admin/"), request);
}

void KCupsRequest::setDefaultPrinter(const QString &name)
{
    QVariantHash request;
    request["printer-name"] = name;
    doOperation(CUPS_SET_DEFAULT, QLatin1String("/admin/"), request);
}

void KCupsRequest::deletePrinter(const QString &name)
{
    QVariantHash request;
    request["printer-name"] = name;
    doOperation(CUPS_DELETE_PRINTER, QLatin1String("/admin/"), request);
}

void KCupsRequest::printTestPage(const QString &printer, bool isClass)
{
    QVariantHash request;
    request["printer-name"] = printer;
    request["printer-is-class"] = isClass;
    request["job-name"] = i18n("Test Page");
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
        resource = QLatin1String("/classes/") % printer;
    } else {
        resource = QLatin1String("/printers/") % printer;
    }

    doOperation(IPP_PRINT_JOB, resource, request);
}

void KCupsRequest::printCommand(const QString &printer, const QString &command, const QString &title)
{
    if (KCupsConnection::readyToStart()) {
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
                                        printer.toUtf8(),
                                        title.toUtf8(),
                                        1,
                                        &hold_option)) < 1) {
                qWarning() << "Unable to send command to printer driver!";

                setError(IPP_NOT_POSSIBLE, i18n("Unable to send command to printer driver!"));
                setFinished();
                return;
            }

            status = cupsStartDocument(CUPS_HTTP_DEFAULT,
                                       printer.toUtf8(),
                                       job_id,
                                       NULL,
                                       CUPS_FORMAT_COMMAND,
                                       1);
            if (status == HTTP_CONTINUE) {
                status = cupsWriteRequestData(CUPS_HTTP_DEFAULT, command_file,
                                            strlen(command_file));
            }

            if (status == HTTP_CONTINUE) {
                cupsFinishDocument(CUPS_HTTP_DEFAULT, printer.toUtf8());
            }

            setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
            if (KCupsConnection::lastError() >= IPP_REDIRECTION_OTHER_SITE) {
                qWarning() << "Unable to send command to printer driver!";

                cupsCancelJob(printer.toUtf8(), job_id);
                setFinished();
                return; // Return to avoid a new try
            }
        } while (KCupsConnection::retryIfForbidden());
        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("printCommand", printer, command, title);
    }
}

void KCupsRequest::cancelJob(const QString &destName, int jobId)
{
    QVariantHash request;
    request["printer-name"] = destName;
    request["job-id"] = jobId;

    doOperation(IPP_CANCEL_JOB, QLatin1String("/jobs/"), request);
}

void KCupsRequest::holdJob(const QString &destName, int jobId)
{
    QVariantHash request;
    request["printer-name"] = destName;
    request["job-id"] = jobId;

    doOperation(IPP_HOLD_JOB, QLatin1String("/jobs/"), request);
}

void KCupsRequest::releaseJob(const QString &destName, int jobId)
{
    QVariantHash request;
    request["printer-name"] = destName;
    request["job-id"] = jobId;

    doOperation(IPP_RELEASE_JOB, QLatin1String("/jobs/"), request);
}

void KCupsRequest::moveJob(const QString &fromDestname, int jobId, const QString &toDestname)
{
    if (jobId < -1 || fromDestname.isEmpty() || toDestname.isEmpty() || jobId == 0) {
        qWarning() << "Internal error, invalid input data" << jobId << fromDestname << toDestname;
        setFinished();
        return;
    }

    QVariantHash request;
    request["printer-name"] = fromDestname;
    request["job-id"] = jobId;
    request["job-printer-uri"] = toDestname;

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
    m_error = false;
    m_errorMsg.clear();
    m_printers.clear();
    m_jobs.clear();
    m_retArguments.clear();

    // If this fails we get into a infinite loop
    // Do not use global()->thread() which point
    // to the KCupsConnection parent thread
    moveToThread(KCupsConnection::global());

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
        setError(1, i18n("Failed to invoke method: %1", method));
        setFinished();
    }
}

void KCupsRequest::doOperation(int operation, const QString &resource, const QVariantHash &request)
{
    if (KCupsConnection::readyToStart()) {
        KCupsConnection::request(static_cast<ipp_op_e>(operation),
                                 resource,
                                 request,
                                 false);

        setError(KCupsConnection::lastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("doOperation", operation, resource, request);
    }
}

ReturnArguments KCupsRequest::result() const
{
    return m_retArguments;
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

int KCupsRequest::error() const
{
    return m_error;
}

QString KCupsRequest::errorMsg() const
{
    return m_errorMsg;
}

void KCupsRequest::setError(int error, const QString &errorMsg)
{
    m_error = error;
    m_errorMsg = errorMsg;
}

void KCupsRequest::setFinished()
{
    m_finished = true;
    emit finished();
}
