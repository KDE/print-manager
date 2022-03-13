/**
 * SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "printer.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <KAboutData>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KPluginFactory>

#include "PrinterModel.h"

#include <KMessageBox>

K_PLUGIN_CLASS_WITH_JSON(Printer, "kcm_printer_manager.json")

Printer::Printer(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, metaData, args)
{
    setButtons(KQuickAddons::ConfigModule::NoAdditionalButton);

    // Make sure we update our server settings if the user change it on
    // another interface
    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &Printer::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &Printer::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &Printer::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &Printer::getServerSettings);

    getServerSettings();

    auto model = new PrinterModel(this);
    m_model = new PrinterSortFilterModel(this);
    m_model->setSourceModel(model);
}

PrinterSortFilterModel *Printer::printerModel() const
{
    return m_model;
}

void Printer::addPrinter()
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("kde-add-printer"), {QStringLiteral("--add-printer")});
    job->start();
}

void Printer::removePrinter(const QString &name)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::requestFinished);
    request->deletePrinter(name);
}

void Printer::configurePrinter(const QString &name)
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("configure-printer"), {name});
    job->start();
}

void Printer::openPrintQueue(const QString &name)
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("kde-print-queue"), {name});
    job->start();
}

void Printer::makePrinterDefault(const QString &name)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::requestFinished);
    request->setDefaultPrinter(name);
}

void Printer::requestFinished(KCupsRequest *request)
{
    if (request->hasError()) {
        Q_EMIT requestError(i18n("Failed to perform request: %1", request->errorMsg()));
    }
}

void Printer::getServerSettings()
{
    auto serverRequest = new KCupsRequest;

    connect(serverRequest, &KCupsRequest::finished, this, [this](KCupsRequest *request) {
        // When we don't have any destinations error is set to IPP_NOT_FOUND
        // we can safely ignore the error since it DOES bring the server
        // settings
        bool error = request->hasError() && request->error() != IPP_NOT_FOUND;

        if (error) {
            if (request->property("interactive").toBool()) {
                KMessageBox::detailedSorry(nullptr, i18nc("@info", "Failed to get server settings"), request->errorMsg(), i18nc("@title:window", "Failed"));
            }
        } else {
            KCupsServer server = request->serverSettings();

            m_shareConnectedPrinters = server.sharePrinters();
            m_allowPrintingFromInternet = server.allowPrintingFromInternet();
            m_allowRemoteAdmin = server.allowRemoteAdmin();
            m_allowUserCancelAnyJob = server.allowUserCancelAnyJobs();
        }

        Q_EMIT settingsChanged();

        request->deleteLater();
    });

    serverRequest->getServerSettings();
}

void Printer::updateServerFinished(KCupsRequest *request)
{
    if (request->hasError()) {
        if (request->error() == IPP_SERVICE_UNAVAILABLE || request->error() == IPP_INTERNAL_ERROR || request->error() == IPP_AUTHENTICATION_CANCELED) {
            // Server is restarting, or auth was canceled, update the settings in one second
            QTimer::singleShot(1000, this, &Printer::getServerSettings);
        } else {
            KMessageBox::detailedSorry(nullptr, i18nc("@info", "Failed to configure server settings"), request->errorMsg(), request->serverError());

            // Force the settings to be retrieved again
            getServerSettings();
        }
    }
    request->deleteLater();
}

bool Printer::shareConnectedPrinters() const
{
    return m_shareConnectedPrinters;
}

void Printer::setShareConnectedPrinters(bool share)
{
    KCupsServer server;
    server.setSharePrinters(share);
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::updateServerFinished);
    request->setServerSettings(server);
}

bool Printer::allowPrintingFromInternet() const
{
    return m_allowPrintingFromInternet;
}

void Printer::setAllowPrintingFromInternet(bool allow)
{
    KCupsServer server;
    server.setAllowPrintingFromInternet(allow);
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::updateServerFinished);
    request->setServerSettings(server);
}

bool Printer::allowRemoteAdmin() const
{
    return m_allowRemoteAdmin;
}

void Printer::setAllowRemoteAdmin(bool allow)
{
    KCupsServer server;
    server.setAllowRemoteAdmin(allow);
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::updateServerFinished);
    request->setServerSettings(server);
}

bool Printer::allowUserCancelAnyJobs() const
{
    return m_allowUserCancelAnyJob;
}

void Printer::setAllowUserCancelAnyJobs(bool allow)
{
    KCupsServer server;
    server.setAllowUserCancelAnyJobs(allow);
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::updateServerFinished);
    request->setServerSettings(server);
}

void Printer::makePrinterShared(const QString &name, bool shared, bool isClass)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::requestFinished);
    request->setShared(name, isClass, shared);
}

void Printer::makePrinterRejectJobs(const QString &name, bool reject)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::requestFinished);

    if (reject) {
        request->rejectJobs(name);
    } else {
        request->acceptJobs(name);
    }
}

void Printer::printTestPage(const QString &name, bool isClass)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::requestFinished);
    request->printTestPage(name, isClass);
}

void Printer::printSelfTestPage(const QString &name)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::requestFinished);
    request->printCommand(name, QLatin1String("PrintSelfTestPage"), i18n("Print Self-Test Page"));
}

void Printer::cleanPrintHeads(const QString &name)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &Printer::requestFinished);
    request->printCommand(name, QLatin1String("Clean all"), i18n("Clean Print Heads"));
}

#include "printer.moc"
