/**
 * SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "printermanager.h"
#include "pmkcm_log.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <KAboutData>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>

#include <PrinterModel.h>
#include <ProcessRunner.h>

using namespace Qt::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(PrinterManager, "kcm_printer_manager.json")

PrinterManager::PrinterManager(QObject *parent, const KPluginMetaData &metaData)
    : KQuickConfigModule(parent, metaData)
{
    setButtons(KQuickConfigModule::NoAdditionalButton);

    // Make sure we update our server settings if the user changes anything on
    // another interface
    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &PrinterManager::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &PrinterManager::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &PrinterManager::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &PrinterManager::getServerSettings);

    getServerSettings();

    auto model = new PrinterModel(this);
    m_model = new PrinterSortFilterModel(this);
    m_model->setSourceModel(model);
}

PrinterSortFilterModel *PrinterManager::printerModel() const
{
    return m_model;
}

void PrinterManager::pausePrinter(const QString &name)
{
    const auto request = setupRequest();
    request->pausePrinter(name);
}

void PrinterManager::resumePrinter(const QString &name)
{
    const auto request = setupRequest();
    request->resumePrinter(name);
}

void PrinterManager::addPrinter()
{
    ProcessRunner::addPrinter();
}

KCupsRequest *PrinterManager::setupRequest() {
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterManager::requestFinished);
    return request;
}

KCupsRequest *PrinterManager::setupServerRequest() {
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrinterManager::updateServerFinished);
    return request;
}

void PrinterManager::removePrinter(const QString &name)
{
    m_removing = true;
    m_saveCount = m_model->rowCount();
    const auto request = setupRequest();
    request->deletePrinter(name);
}

void PrinterManager::configurePrinter(const QString &name)
{
    ProcessRunner::configurePrinter(name);
}

void PrinterManager::openPrintQueue(const QString &name)
{
    ProcessRunner::openPrintQueue(name);
}

void PrinterManager::makePrinterDefault(const QString &name)
{
    const auto request = setupRequest();
    request->setDefaultPrinter(name);
}

void PrinterManager::requestFinished(KCupsRequest *request)
{
    if (request->hasError()) {
        Q_EMIT requestError(i18n("Failed to perform request: %1", request->errorMsg()));
    }
    request->deleteLater();
    
    if (m_removing) {
        m_removing = false;
        Q_EMIT removeComplete(m_model->rowCount() < m_saveCount);
    }
}

void PrinterManager::getServerSettings()
{
    auto serverRequest = new KCupsRequest;

    connect(serverRequest, &KCupsRequest::finished, this, [this](KCupsRequest *request) {
        // When we don't have any destinations error is set to IPP_NOT_FOUND
        // we can safely ignore the error since it DOES bring the server
        // settings
        bool error = request->hasError() && request->error() != IPP_NOT_FOUND;

        if (error) {
            if (request->property("interactive").toBool()) {
                KMessageBox::detailedError(nullptr, i18nc("@info", "Failed to get server settings"), request->errorMsg(), i18nc("@title:window", "Failed"));
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

void PrinterManager::updateServerFinished(KCupsRequest *request)
{
    if (request->hasError()) {
        if (request->error() == IPP_SERVICE_UNAVAILABLE || request->error() == IPP_INTERNAL_ERROR || request->error() == IPP_AUTHENTICATION_CANCELED) {
            // Server is restarting, or auth was canceled, update the settings in one second
            QTimer::singleShot(1000, this, &PrinterManager::getServerSettings);
        } else {
            KMessageBox::detailedError(nullptr, i18nc("@info", "Failed to configure server settings"), request->errorMsg(), request->serverError());

            // Force the settings to be retrieved again
            getServerSettings();
        }
    }
    request->deleteLater();
}

bool PrinterManager::shareConnectedPrinters() const
{
    return m_shareConnectedPrinters;
}

void PrinterManager::setShareConnectedPrinters(bool share)
{
    KCupsServer server;
    server.setSharePrinters(share);
    const auto request = setupServerRequest();
    request->setServerSettings(server);
}

bool PrinterManager::allowPrintingFromInternet() const
{
    return m_allowPrintingFromInternet;
}

void PrinterManager::setAllowPrintingFromInternet(bool allow)
{
    KCupsServer server;
    server.setAllowPrintingFromInternet(allow);
    const auto request = setupServerRequest();
    request->setServerSettings(server);
}

bool PrinterManager::allowRemoteAdmin() const
{
    return m_allowRemoteAdmin;
}

void PrinterManager::setAllowRemoteAdmin(bool allow)
{
    KCupsServer server;
    server.setAllowRemoteAdmin(allow);
    const auto request = setupServerRequest();
    request->setServerSettings(server);
}

bool PrinterManager::allowUserCancelAnyJobs() const
{
    return m_allowUserCancelAnyJob;
}

void PrinterManager::setAllowUserCancelAnyJobs(bool allow)
{
    KCupsServer server;
    server.setAllowUserCancelAnyJobs(allow);
    const auto request = setupServerRequest();
    request->setServerSettings(server);
}

void PrinterManager::makePrinterShared(const QString &name, bool shared, bool isClass)
{
    const auto request = setupRequest();
    request->setShared(name, isClass, shared);
}

void PrinterManager::makePrinterRejectJobs(const QString &name, bool reject)
{
    const auto request = setupRequest();

    if (reject) {
        request->rejectJobs(name);
    } else {
        request->acceptJobs(name);
    }
}

void PrinterManager::printTestPage(const QString &name, bool isClass)
{
    const auto request = setupRequest();
    request->printTestPage(name, isClass);
}

void PrinterManager::printSelfTestPage(const QString &name)
{
    const auto request = setupRequest();
    request->printCommand(name, "PrintSelfTestPage"_L1, i18n("Print Self-Test Page"));
}

void PrinterManager::cleanPrintHeads(const QString &name)
{
    const auto request = setupRequest();
    request->printCommand(name, "Clean all"_L1, i18n("Clean Print Heads"));
}

#include "printermanager.moc"
