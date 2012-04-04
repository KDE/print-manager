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

#include "NewPrinterNotification.h"
#include "newprinternotificationadaptor.h"

#include <KLocale>
#include <KGenericFactory>
#include <KNotification>
#include <KIcon>

#include <KCupsRequest.h>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusServiceWatcher>

K_PLUGIN_FACTORY(PrintDFactory, registerPlugin<NewPrinterNotification>();)
K_EXPORT_PLUGIN(PrintDFactory("printmanager"))

#define STATUS_SUCCESS        0
#define STATUS_MODEL_MISMATCH 1
#define STATUS_GENERIC_DRIVER 2
#define STATUS_NO_DRIVER      3

NewPrinterNotification::NewPrinterNotification(QObject *parent, const QVariantList &args) :
    KDEDModule(parent)
{
    // There's not much use for args in a KCM
    Q_UNUSED(args)

    // Creates or new adaptor
    (void) new NewPrinterNotificationAdaptor(this);

    // Register the com.redhat.NewPrinterNotification interface
    if (!registerService()) {
        // in case registration fails due to another user or application running
        // keep an eye on it so we can register when available
        QDBusServiceWatcher *watcher;
        watcher = new QDBusServiceWatcher(QLatin1String("com.redhat.NewPrinterNotification"),
                                          QDBusConnection::systemBus(),
                                          QDBusServiceWatcher::WatchForUnregistration,
                                          this);
        connect(watcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(registerService()));
    }

}

NewPrinterNotification::~NewPrinterNotification()
{
}

void NewPrinterNotification::GetReady()
{
    kDebug();
    // This method is all about telling the user a new printer was detected
    KNotification *notify = new KNotification("GetReady");
    notify->setComponentData(KComponentData("printmanager"));
    notify->setPixmap(KIcon("printer").pixmap(64, 64));
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
                                        const QString &mfg,
                                        const QString &mdl,
                                        const QString &des,
                                        const QString &cmd)
{
    kDebug() << status << name << mfg << mdl << des << cmd;
    // This method is all about telling the user a new printer was detected
    KNotification *notify = new KNotification("NewPrinterNotification");
    notify->setComponentData(KComponentData("printmanager"));
    notify->setPixmap(KIcon("printer").pixmap(64, 64));
    if (status < STATUS_GENERIC_DRIVER) {
        notify->setTitle(i18n("The New Printer was Added"));
    } else {
        notify->setTitle(i18n("The New Printer is Missing Drivers"));
    }

    if (status == STATUS_SUCCESS) {
        // TODO isn't mdl a better string?
        notify->setText(i18n("'%1' is ready for printing.", name));
    } else {
        QString driver;
        KCupsRequest *request = new KCupsRequest;
        request->getPrinterAttributes(name, false, KCupsPrinter::PrinterMakeAndModel);
        request->waitTillFinished();
        if (!request->printers().isEmpty()){
            KCupsPrinter printer = request->printers().first();
            driver = printer.makeAndModel();
        }
        request->deleteLater();

        // The cups request might have failed
        if (driver.isEmpty()) {
            notify->setText(i18n("'%1' has been added, please check its driver.", name));
        } else {
            connect(notify, SIGNAL(activated(uint)), this, SLOT(configurePrinter()));
            notify->setProperty("PrinterName", name);
            notify->setFlags(KNotification::Persistent);
            notify->setActions(QStringList() << i18n("Configure"));
            notify->setText(i18n("'%1' has been added, using the '%2' driver.", name, driver));
        }
    }
    notify->sendEvent();
}

bool NewPrinterNotification::registerService()
{
    if (!QDBusConnection::systemBus().registerService("com.redhat.NewPrinterNotification")) {
        kDebug() << "unable to register service to dbus";
        return false;
    }

    if (!QDBusConnection::systemBus().registerObject("/com/redhat/NewPrinterNotification", this)) {
        kDebug() << "unable to register object to dbus";
        return false;
    }
    return true;
}

void NewPrinterNotification::configurePrinter()
{
    if (sender()) {
        QString printerName;
        printerName = sender()->property("PrinterName").toString();
        QDBusMessage message;
        message = QDBusMessage::createMethodCall(QLatin1String("org.kde.ConfigurePrinter"),
                                                 QLatin1String("/"),
                                                 QLatin1String("org.kde.ConfigurePrinter"),
                                                 QLatin1String("ConfigurePrinter"));
        // Use our own cached tid to avoid crashes
        message << qVariantFromValue(printerName);
        QDBusConnection::sessionBus().send(message);
    }
}
