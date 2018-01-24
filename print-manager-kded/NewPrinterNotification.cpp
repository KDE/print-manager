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
#include <KToolInvocation>

#include <KCupsRequest.h>

#include <QIcon>
#include <QThread>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusReply>

#define STATUS_SUCCESS        0
#define STATUS_MODEL_MISMATCH 1
#define STATUS_GENERIC_DRIVER 2
#define STATUS_NO_DRIVER      3

#define PRINTER_NAME "PrinterName"
#define DEVICE_ID    "DeviceId"

NewPrinterNotification::NewPrinterNotification()
{
    // Make sure the password dialog is created in the main thread
    KCupsConnection::global();

    // Make all our init code run on the thread since
    // the DBus calls were made blocking
    QTimer::singleShot(0, this, &NewPrinterNotification::init);

    m_thread = new QThread(this);
    moveToThread(m_thread);
    m_thread->start();
}

NewPrinterNotification::~NewPrinterNotification()
{
}

void NewPrinterNotification::GetReady()
{
    qCDebug(PM_KDED);
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

    QString title;
    QString text;
    QString devid;
    QStringList actions;
    devid = QString::fromLatin1("MFG:%1;MDL:%2;DES:%3;CMD:%4;").arg(make, model, description, cmd);

    if (name.contains(QLatin1Char('/'))) {
        // name is a URI, no queue was generated, because no suitable
        // driver was found
        title = i18n("Missing printer driver");
        if (!make.isEmpty() && !model.isEmpty()) {
            text = i18n("No printer driver for %1 %2.", make, model);
        } else if (!description.isEmpty()) {
            text = i18n("No printer driver for %1.", description);
        } else {
            text = i18n("No driver for this printer.");
        }

        actions << i18n("Search");
        connect(notify, &KNotification::action1Activated, this, &NewPrinterNotification::setupPrinter);
    } else {
        // name is the name of the queue which hal_lpadmin has set up
        // automatically.

        if (status < STATUS_GENERIC_DRIVER) {
            title = i18n("The New Printer was Added");
        } else {
            title = i18n("The New Printer is Missing Drivers");
        }

        QStringList attr;
        attr << KCUPS_PRINTER_MAKE_AND_MODEL;

        // Get the new printer attributes
        QPointer<KCupsRequest> request = new KCupsRequest;
        request->getPrinterAttributes(name, false, attr);
        request->waitTillFinished();
        if (!request) {
            return;
        }

        QString driver;
        // Get the new printer driver
        if (!request->printers().isEmpty()){
            KCupsPrinter printer = request->printers().first();
            driver = printer.makeAndModel();
        }
        request->deleteLater();

        QString ppdFileName;
        request = new KCupsRequest;
        request->getPrinterPPD(name);
        request->waitTillFinished();
        if (!request) {
            return;
        }
        ppdFileName = request->printerPPD();
        request->deleteLater();

        // Get a list of missing executables
        QStringList missingExecutables = getMissingExecutables(ppdFileName);

        if (!missingExecutables.isEmpty()) {
            // TODO check with PackageKit about missing drivers
            qCWarning(PM_KDED) << "Missing executables:" << missingExecutables;
        } else if (status == STATUS_SUCCESS) {
            text = i18n("'%1' is ready for printing.", name);
            actions << i18n("Print test page");
            connect(notify, &KNotification::action1Activated, this, &NewPrinterNotification::printTestPage);
            actions << i18n("Configure");
            connect(notify, &KNotification::action2Activated, this, &NewPrinterNotification::configurePrinter);
        } else {
            // Model mismatch

            // The cups request might have failed
            if (driver.isEmpty()) {
                text = i18n("'%1' has been added, please check its driver.", name);
                actions << i18n("Configure");
                connect(notify, &KNotification::action1Activated, this, &NewPrinterNotification::configurePrinter);
            } else {
                text = i18n("'%1' has been added, using the '%2' driver.", name, driver);
                actions << i18n("Print test page");
                connect(notify, &KNotification::action1Activated, this, &NewPrinterNotification::printTestPage);
                actions << i18n("Find driver");
                connect(notify, &KNotification::action2Activated, this, &NewPrinterNotification::findDriver);
            }
        }
    }
    notify->setTitle(title);
    notify->setText(text);
    notify->setProperty(PRINTER_NAME, name);
    notify->setProperty(DEVICE_ID, devid);
    notify->setActions(actions);
    notify->sendEvent();
}

void NewPrinterNotification::init()
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
    QProcess::startDetached(QLatin1String("configure-printer"), { QLatin1String(PRINTER_NAME) });
}

void NewPrinterNotification::searchDrivers()
{
}

void NewPrinterNotification::printTestPage()
{
    qCDebug(PM_KDED);
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->printTestPage(sender()->property(PRINTER_NAME).toString(), false);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void NewPrinterNotification::findDriver()
{
    qCDebug(PM_KDED);
    // This function will show the PPD browser dialog
    // to choose a better PPD to the already added printer
    QStringList args;
    args << QLatin1String("--change-ppd");
    args << sender()->property(PRINTER_NAME).toString();
    KToolInvocation::kdeinitExec(QLatin1String("kde-add-printer"), args);
}

void NewPrinterNotification::installDriver()
{
    qCDebug(PM_KDED);
}

void NewPrinterNotification::setupPrinter()
{
    qCDebug(PM_KDED);
    QObject *obj = sender();
    // This function will show the PPD browser dialog
    // to choose a better PPD, queue name, location
    // in this case the printer was not added
    QStringList args{
        QLatin1String("--new-printer-from-device"),
                obj->property(PRINTER_NAME).toString() % QLatin1Char('/') % obj->property(DEVICE_ID).toString()
    };
    KToolInvocation::kdeinitExec(QLatin1String("kde-add-printer"), args);
}

QStringList NewPrinterNotification::getMissingExecutables(const QString &ppdFileName) const
{
    qCDebug(PM_KDED);
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("/org/fedoraproject/Config/Printing"),
                                             QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("MissingExecutables"));
    message << ppdFileName;
    QDBusReply<QStringList> reply = QDBusConnection::sessionBus().call(message);
    if (!reply.isValid()) {
        qCWarning(PM_KDED) << "Invalid reply" << reply.error();
    }
    return reply;
}

#include "moc_NewPrinterNotification.cpp"
