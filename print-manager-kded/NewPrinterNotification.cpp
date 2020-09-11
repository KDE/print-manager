/***************************************************************************
 *   Copyright (C) 2010-2018 by Daniel Nicoletti                           *
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

#include "NewPrinterNotification.h"
#include "newprinternotificationadaptor.h"

#include "Debug.h"

#include <KLocalizedString>
#include <KNotification>
#include <KIO/CommandLauncherJob>

#include <KCupsRequest.h>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>

#define STATUS_SUCCESS        0
#define STATUS_MODEL_MISMATCH 1
#define STATUS_GENERIC_DRIVER 2
#define STATUS_NO_DRIVER      3

#define PRINTER_NAME "PrinterName"

NewPrinterNotification::NewPrinterNotification(QObject *parent) : QObject(parent)
{
    // Creates our new adaptor
    (void) new NewPrinterNotificationAdaptor(this);

    // Register the com.redhat.NewPrinterNotification interface
    if (!registerService()) {
        // in case registration fails due to another user or application running
        // keep an eye on it so we can register when available
        auto watcher = new QDBusServiceWatcher(QLatin1String("com.redhat.NewPrinterNotification"),
                                               QDBusConnection::systemBus(),
                                               QDBusServiceWatcher::WatchForUnregistration,
                                               this);
        connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, &NewPrinterNotification::registerService);
    }
}

NewPrinterNotification::~NewPrinterNotification()
{
}

void NewPrinterNotification::GetReady()
{
    qCDebug(PM_KDED) << "GetReady";
    // This method is all about telling the user a new printer was detected
    auto notify = new KNotification(QLatin1String("GetReady"));
    notify->setComponentName(QLatin1String("printmanager"));
    notify->setIconName(QLatin1String("printer"));
    notify->setTitle(i18n("A New Printer was detected"));
    notify->setText(i18n("Configuring new printer..."));
    notify->sendEvent();
}

//status: 0
//name: PSC_1400_series
//mfg: HP
//mdl: PSC 1400 series
//des:
//cmd: LDL,MLC,PML,DYN
void NewPrinterNotification::NewPrinter(int status,
                                        const QString &name,
                                        const QString &make,
                                        const QString &model,
                                        const QString &description,
                                        const QString &cmd)
{
    qCDebug(PM_KDED) << status << name << make << model << description << cmd;

    // 1
    // "usb://Samsung/SCX-3400%20Series?serial=Z6Y1BQAC500079K&interface=1"
    // mfg "Samsung"
    // mdl "SCX-3400 Series" "" "SPL,FWV,PIC,BDN,EXT"
    // This method is all about telling the user a new printer was detected
    auto notify = new KNotification(QLatin1String("NewPrinterNotification"));
    notify->setComponentName(QLatin1String("printmanager"));
    notify->setIconName(QLatin1String("printer"));
    notify->setFlags(KNotification::Persistent);

    if (name.contains(QLatin1Char('/'))) {
        const QString devid = QString::fromLatin1("MFG:%1;MDL:%2;DES:%3;CMD:%4;")
                .arg(make, model, description, cmd);
        setupPrinterNotification(notify, make, model, description,
                                 name + QLatin1Char('/') + devid);
    } else {
        notify->setProperty(PRINTER_NAME, name);
        // name is the name of the queue which hal_lpadmin has set up
        // automatically.

        if (status < STATUS_GENERIC_DRIVER) {
            notify->setTitle(i18n("The New Printer was Added"));
        } else {
            notify->setTitle(i18n("The New Printer is Missing Drivers"));
        }

        auto request = new KCupsRequest;
        connect(request, &KCupsRequest::finished, this, [this, notify, status, name] (KCupsRequest *request) {
            const QString ppdFileName = request->printerPPD();
            // Get a list of missing executables
            getMissingExecutables(notify, status, name, ppdFileName);
            request->deleteLater();
        });
        request->getPrinterPPD(name);   
    }
}

bool NewPrinterNotification::registerService()
{
    if (!QDBusConnection::systemBus().registerService(QLatin1String("com.redhat.NewPrinterNotification"))) {
        qCWarning(PM_KDED) << "unable to register service to dbus";
        return false;
    }

    if (!QDBusConnection::systemBus().registerObject(QLatin1String("/com/redhat/NewPrinterNotification"), this)) {
        qCWarning(PM_KDED) << "unable to register object to dbus";
        return false;
    }
    return true;
}

void NewPrinterNotification::configurePrinter()
{
    const QString printerName = sender()->property(PRINTER_NAME).toString();
    qCDebug(PM_KDED) << "configure printer tool" << printerName;
    QProcess::startDetached(QLatin1String("configure-printer"), { printerName });
}

void NewPrinterNotification::printTestPage()
{
    const QString printerName = sender()->property(PRINTER_NAME).toString();
    qCDebug(PM_KDED) << "printing test page for" << printerName;

    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, request, &KCupsRequest::deleteLater);
    request->printTestPage(printerName, false);
}

void NewPrinterNotification::findDriver()
{
    const QString printerName = sender()->property(PRINTER_NAME).toString();
    qCDebug(PM_KDED) << "find driver for" << printerName;

    // This function will show the PPD browser dialog
    // to choose a better PPD to the already added printer

    auto job = new KIO::CommandLauncherJob(QStringLiteral("kde-add-printer"), {QStringLiteral("--change-ppd"), printerName});
    job->setDesktopName(QStringLiteral("org.kde.kde-add-printer"));
    job->start();
}

void NewPrinterNotification::setupPrinterNotification(KNotification *notify, const QString &make, const QString &model, const QString &description, const QString &arg)
{
    // name is a URI, no queue was generated, because no suitable
    // driver was found
    notify->setTitle(i18n("Missing printer driver"));
    if (!make.isEmpty() && !model.isEmpty()) {
        notify->setText(i18n("No printer driver for %1 %2.", make, model));
    } else if (!description.isEmpty()) {
        notify->setText(i18n("No printer driver for %1.", description));
    } else {
        notify->setText(i18n("No driver for this printer."));
    }

    notify->setActions({ i18n("Search") });
    connect(notify, &KNotification::action1Activated, this, [notify, arg] () {
        qCDebug(PM_KDED);
        // This function will show the PPD browser dialog
        // to choose a better PPD, queue name, location
        // in this case the printer was not added
        auto job = new KIO::CommandLauncherJob(QStringLiteral("kde-add-printer"), {QLatin1String("--new-printer-from-device"), arg});
        job->setDesktopName(QStringLiteral("org.kde.PrintQueue"));
        job->start();
    });

    notify->sendEvent();
}

void NewPrinterNotification::getMissingExecutables(KNotification *notify, int status, const QString &name, const QString &ppdFileName)
{
    qCDebug(PM_KDED) << "get missing executables" << ppdFileName;
    QDBusMessage message = QDBusMessage::createMethodCall(
                QLatin1String("org.fedoraproject.Config.Printing"),
                QLatin1String("/org/fedoraproject/Config/Printing"),
                QLatin1String("org.fedoraproject.Config.Printing"),
                QLatin1String("MissingExecutables"));
    message << ppdFileName;

    QDBusPendingReply<QStringList> reply = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, notify, status, name] () {
        watcher->deleteLater();
        QDBusPendingReply<QStringList> reply = *watcher;
        if (!reply.isValid()) {
            qCWarning(PM_KDED) << "Invalid reply" << reply.error();
            notify->deleteLater();
            return;
        }

        const QStringList missingExecutables = reply;
        if (!missingExecutables.isEmpty()) {
            // TODO check with PackageKit about missing drivers
            qCWarning(PM_KDED) << "Missing executables:" << missingExecutables;
            notify->deleteLater();
            return;
        } else if (status == STATUS_SUCCESS) {
            printerReadyNotification(notify, name);
        } else {
            // Model mismatch
            checkPrinterCurrentDriver(notify, name);
        }
    });
}

void NewPrinterNotification::checkPrinterCurrentDriver(KNotification *notify, const QString &name)
{
    // Get the new printer attributes
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, notify, name] (KCupsRequest *request) {
        request->deleteLater();

        QString driver;
        // Get the new printer driver
        if (!request->printers().isEmpty()){
            const KCupsPrinter &printer = request->printers().first();
            driver = printer.makeAndModel();
        }

        // The cups request might have failed
        if (driver.isEmpty()) {
            notify->setText(i18n("'%1' has been added, please check its driver.", name));
            notify->setActions({ i18n("Configure") });
            connect(notify, &KNotification::action1Activated, this, &NewPrinterNotification::configurePrinter);
        } else {
            notify->setText(i18n("'%1' has been added, using the '%2' driver.", name, driver));
            notify->setActions({ i18n("Print test page"), i18n("Find driver") });
            connect(notify, &KNotification::action1Activated, this, &NewPrinterNotification::printTestPage);
            connect(notify, &KNotification::action2Activated, this, &NewPrinterNotification::findDriver);
        }
        notify->sendEvent();
    });
    request->getPrinterAttributes(name, false, { KCUPS_PRINTER_MAKE_AND_MODEL });
}

void NewPrinterNotification::printerReadyNotification(KNotification *notify, const QString &name)
{
    notify->setText(i18n("'%1' is ready for printing.", name));
    notify->setActions({ i18n("Print test page"), i18n("Configure") });
    connect(notify, &KNotification::action1Activated, this, &NewPrinterNotification::printTestPage);
    connect(notify, &KNotification::action2Activated, this, &NewPrinterNotification::configurePrinter);
    notify->sendEvent();
}

#include "moc_NewPrinterNotification.cpp"
