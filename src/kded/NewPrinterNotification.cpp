/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NewPrinterNotification.h"
#include "newprinternotificationadaptor.h"

#include "pmkded_log.h"

#include <KLocalizedString>
#include <KNotification>

#include <KCupsRequest.h>
#include <ProcessRunner.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>

using namespace Qt::StringLiterals;

static constexpr int STATUS_SUCCESS = 0;
static constexpr int STATUS_MODEL_MISMATCH = 1;
static constexpr int STATUS_GENERIC_DRIVER = 2;
static constexpr int STATUS_NO_DRIVER = 3;

static constexpr QLatin1String SERVICE = "com.redhat.NewPrinterNotification"_L1;
static constexpr QLatin1String OBJECT  = "/com/redhat/NewPrinterNotification"_L1;

NewPrinterNotification::NewPrinterNotification(QObject *parent)
    : QObject(parent)
{
    // Creates our new adaptor
    new NewPrinterNotificationAdaptor(this);

    // Register the com.redhat.NewPrinterNotification interface
    if (!registerService()) {
        // in case registration fails due to another user or application running
        // keep an eye on it so we can register when available
        auto watcher = new QDBusServiceWatcher(SERVICE,
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
    qCDebug(PMKDED) << Q_FUNC_INFO;

    // We get an unnecessary notification from the device manager
    // so dismiss it before sending ours
    QDBusMessage msg = QDBusMessage::createMethodCall(u"org.kde.kded6"_s,
                                                      u"/modules/devicenotifications"_s,
                                                      u"org.kde.plasma.devicenotifications"_s,
                                                      u"dismissUsbDeviceAdded"_s);
    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);

    auto notify = new KNotification(QLatin1String("GetReady"));
    notify->setComponentName(QLatin1String("printmanager"));
    notify->setTitle(i18n("A new printer was detected"));
    const auto defAction = notify->addAction(i18nc("@action:button", "Configure new printer…"));
    connect(defAction, &KNotificationAction::activated, notify, [notify]() {
        ProcessRunner::addPrinter(notify->xdgActivationToken().toUtf8());
    });
    notify->sendEvent();
}

// status: 0
// name: PSC_1400_series
// mfg: HP
// mdl: PSC 1400 series
// des:
// cmd: LDL,MLC,PML,DYN
void NewPrinterNotification::NewPrinter(int status,
                                        const QString &name,
                                        const QString &make,
                                        const QString &model,
                                        const QString &description,
                                        const QString &cmd)
{
    qCDebug(PMKDED) << status << name << make << model << description << cmd;

    // 1
    // "usb://Samsung/SCX-3400%20Series?serial=Z6Y1BQAC500079K&interface=1"
    // mfg "Samsung"
    // mdl "SCX-3400 Series" "" "SPL,FWV,PIC,BDN,EXT"
    // This method is all about telling the user a new printer was detected
    auto notify = new KNotification(QLatin1String("NewPrinterNotification"));
    notify->setComponentName(QLatin1String("printmanager"));

    if (name.contains(QLatin1Char('/'))) {
        // name is a uri
        const QString devid = QString::fromLatin1("MFG:%1;MDL:%2;DES:%3;CMD:%4;").arg(make, model, description, cmd);
        notifyQueueNotCreated(notify, make, model, description, name + QLatin1Char('/') + devid);
    } else {
        // name is the name of the queue which hal_lpadmin has set up
        // automatically.
        if (status < STATUS_GENERIC_DRIVER) {
            notify->setTitle(i18n("The New Printer was added"));
        } else {
            notify->setTitle(i18n("The New Printer might have a driver problem"));
        }

        auto request = new KCupsRequest;
        connect(request, &KCupsRequest::finished, this, [this, notify, status, name](KCupsRequest *request) {
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
    if (!QDBusConnection::systemBus().registerService(SERVICE)) {
        qCWarning(PMKDED) << "Unable to register service to systemBus:" << SERVICE;
        return false;
    }
    qCDebug(PMKDED) << SERVICE << "registered to systemBus";

    if (!QDBusConnection::systemBus().registerObject(OBJECT, this)) {
        qCWarning(PMKDED) << "Unable to register object to systemBus:" << OBJECT;
        return false;
    }
    qCDebug(PMKDED) << OBJECT << "registered to systemBus";

    return true;
}

void NewPrinterNotification::printTestPage(const QString &printerName)
{
    qCDebug(PMKDED) << "printing test page for" << printerName;
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, request, &KCupsRequest::deleteLater);
    request->printTestPage(printerName, false);
}

void NewPrinterNotification::notifyQueueNotCreated(KNotification *notify,
                                                      const QString &make,
                                                      const QString &model,
                                                      const QString &description,
                                                      const QString &arg)
{
    Q_UNUSED(arg)
    // name is a URI, no queue was generated, because no suitable
    // driver was found, offer to add device in notification
    notify->setTitle(i18n("Printer queue was not created"));
    if (!make.isEmpty() && !model.isEmpty()) {
        notify->setText(i18n("Printer driver not found for %1 %2", make, model));
    } else if (!description.isEmpty()) {
        notify->setText(i18n("Printer driver not found for %1", description));
    } else {
        notify->setText(i18n("Printer driver not found for this printer"));
    }
    auto addAction = notify->addAction(i18n("Add Printer…"));
    connect(addAction, &KNotificationAction::activated, notify, [notify]() {
        ProcessRunner::addPrinter(notify->xdgActivationToken().toUtf8());
    });
    notify->sendEvent();
}

void NewPrinterNotification::getMissingExecutables(KNotification *notify, int status, const QString &name, const QString &ppdFileName)
{
    qCDebug(PMKDED) << "get missing executables" << ppdFileName;
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.fedoraproject.Config.Printing"),
                                                          QLatin1String("/org/fedoraproject/Config/Printing"),
                                                          QLatin1String("org.fedoraproject.Config.Printing"),
                                                          QLatin1String("MissingExecutables"));
    message << ppdFileName;

    QDBusPendingReply<QStringList> reply = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, notify, status, name]() {
        watcher->deleteLater();
        QDBusPendingReply<QStringList> reply = *watcher;
        if (!reply.isValid()) {
            qCWarning(PMKDED) << "Invalid reply" << reply.error();
            notify->deleteLater();
            return;
        }

        const QStringList missingExecutables = reply;
        if (!missingExecutables.isEmpty()) {
            // TODO check with PackageKit about missing drivers
            notify->setText(missingExecutables.join(QLatin1Char(' ')));
            notify->sendEvent();
        } else if (status == STATUS_SUCCESS) {
            notifyReady(notify, name);
        } else {
            // Model mismatch
            notifyDriverCheck(notify, name);
        }
    });
}

void NewPrinterNotification::notifyDriverCheck(KNotification *notify, const QString &name)
{
    // Get the new printer attributes
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, notify, name](KCupsRequest *request) {
        request->deleteLater();

        QString driver;
        // Get the new printer driver
        const auto printers = request->printers();
        if (!printers.isEmpty()) {
            driver = printers.first().makeAndModel();
        }

        // The cups request might have failed
        if (driver.isEmpty()) {
            notify->setText(i18n("'%1' has been added, please check its driver", name));
            auto configAction = notify->addAction(i18n("Configure"));
            connect(configAction, &KNotificationAction::activated, notify, [name, notify]() {
                ProcessRunner::kcmConfigurePrinter(name, notify->xdgActivationToken().toUtf8());
            });
        } else {
            notify->setText(i18n("'%1' has been added, using the '%2' driver", name, driver));

            auto testAction = notify->addAction(i18n("Print test page"));
            connect(testAction, &KNotificationAction::activated, notify, [this, name]() {
                printTestPage(name);
            });

            auto findAction = notify->addAction(i18n("Check driver"));
            connect(findAction, &KNotificationAction::activated, this, [name, notify]() {
                ProcessRunner::kcmConfigurePrinter(name, notify->xdgActivationToken().toUtf8());
            });
        }
        notify->sendEvent();
    });
    request->getPrinterAttributes(name, false, {KCUPS_PRINTER_MAKE_AND_MODEL});
}

void NewPrinterNotification::notifyReady(KNotification *notify, const QString &name)
{
    notify->setText(i18n("'%1' is ready for printing.", name));

    auto testAction = notify->addAction(i18n("Print test page"));
    connect(testAction, &KNotificationAction::activated, notify, [this, name]() {
        printTestPage(name);
    });

    auto configAction = notify->addAction(i18n("Configure"));
    connect(configAction, &KNotificationAction::activated, notify, [name, notify]() {
        ProcessRunner::kcmConfigurePrinter(name, notify->xdgActivationToken().toUtf8());
    });

    notify->sendEvent();
}

#include "moc_NewPrinterNotification.cpp"
