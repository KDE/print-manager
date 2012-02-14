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

#include "KCupsRequestServer.h"

#include "KCupsPrinter.h"
#include "KCupsServer.h"

#include <KLocale>
#include <KDebug>

#include <cups/adminutil.h>

KCupsRequestServer::KCupsRequestServer()
{
    qRegisterMetaType<KCupsPrinter>("KCupsPrinter");
    qRegisterMetaType<KCupsServer>("KCupsServer");
}

QString KCupsRequestServer::serverError() const
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

void KCupsRequestServer::getPPDS(const QString &make)
{
    if (KCupsConnection::readyToStart()) {
        Arguments request;
        if (!make.isEmpty()){
            request["ppd-make-and-model"] = make;
        }
        request["need-dest-name"] = false;

        m_retArguments = KCupsConnection::request(CUPS_GET_PPDS,
                                                  "/",
                                                  request,
                                                  true);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getDevices");
    }
}

static void
choose_device_cb(
    const char *device_class,           /* I - Class */
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
    KCupsRequestServer *request = static_cast<KCupsRequestServer*>(user_data);
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

void KCupsRequestServer::getDevices()
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
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getDevices");
    }
}

// THIS function can get the default server dest through the
// "printer-is-default" attribute BUT it does not get user
// defined default printer, see cupsGetDefault() on www.cups.org for details
void KCupsRequestServer::getPrinters(const QStringList &requestedAttr)
{
    if (KCupsConnection::readyToStart()) {
        Arguments request;
        request["printer-type"] = CUPS_PRINTER_LOCAL;
//        if (mask >= 0) {
//        cups_ptype_e mask,
//            request["printer-type-mask"] = mask;
//        }
        request["requested-attributes"] = requestedAttr;
        request["need-dest-name"] = true;

        ReturnArguments dests;
        dests = KCupsConnection::request(CUPS_GET_PRINTERS,
                                                  "/",
                                                  request,
                                                  true);

        for (int i = 0; i < dests.size(); i++) {
            emit printer(i, KCupsPrinter(dests.at(i)));
        }

        m_retArguments = dests;
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinters", requestedAttr);
    }
}

void KCupsRequestServer::getJobs(const QString &printer, bool myJobs, int whichJobs, const QStringList &requestedAttr)
{
    if (KCupsConnection::readyToStart()) {
        Arguments request;
        if (printer.isEmpty()) {
            request["printer-uri"] = printer;
        } else {
            request["printer-name"] = printer;
        }

        if (myJobs) {
            request["my-jobs"] = myJobs;
        }

        if (whichJobs == CUPS_WHICHJOBS_COMPLETED) {
            request["which-jobs"] = "completed";
        } else if (whichJobs == CUPS_WHICHJOBS_ALL) {
            request["which-jobs"] = "all";
        }

        if (!requestedAttr.isEmpty()) {
            request["requested-attributes"] = requestedAttr;
        }
        request["group-tag-qt"] = IPP_TAG_JOB;

        m_retArguments = KCupsConnection::request(IPP_GET_JOBS,
                                                  "/",
                                                  request,
                                                  true);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobs", printer, myJobs, whichJobs, requestedAttr);
    }
}

void KCupsRequestServer::addClass(const QHash<QString, QVariant> &values)
{
//    if (values.isEmpty()) {
//        return 0;
//    }

    if (KCupsConnection::readyToStart()) {
        Arguments request = values;
        request["printer-is-class"] = true;
        request["printer-is-accepting-jobs"] = true;
        request["printer-state"] = IPP_PRINTER_IDLE;
        m_retArguments = KCupsConnection::request(CUPS_ADD_MODIFY_CLASS,
                                                  "/admin/",
                                                  request,
                                                  false);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("addClass", values);
    }
}

void KCupsRequestServer::getServerSettings()
{
    if (KCupsConnection::readyToStart()) {
        do {
            int num_settings;
            cups_option_t *settings;
            Arguments arguments;
            cupsAdminGetServerSettings(CUPS_HTTP_DEFAULT, &num_settings, &settings);
            for (int i = 0; i < num_settings; ++i) {
                QString name = QString::fromUtf8(settings[i].name);
                QString value = QString::fromUtf8(settings[i].value);
                arguments[name] = value;
            }
            cupsFreeOptions(num_settings, settings);

            emit server(KCupsServer(arguments));
        } while (KCupsConnection::retryIfForbidden());
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getServerSettings");
    }
}

void KCupsRequestServer::setServerSettings(const KCupsServer &server)
{
    if (KCupsConnection::readyToStart()) {
        do {
            Arguments args = server.arguments();
            int num_settings = 0;
            cups_option_t *settings;

            Arguments::const_iterator i = args.constBegin();
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
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setServerSettings", qVariantFromValue(server));
    }
}
