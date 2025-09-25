/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrintQueue.h"
#include "pmqueue_log.h"

#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>

#include <KCupsPrinter.h>
#include <KCupsRequest.h>

#include <KIO/AuthInfo>
#include <KJob>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <KPasswdServerClient>
#include <KUserTimestamp>

using namespace Qt::StringLiterals;

PrintQueue::PrintQueue(const QString &printerName, QObject *parent)
    : QObject(parent)
    , m_engine(new QQmlApplicationEngine)
{
    m_engine->rootContext()->setContextObject(new KLocalizedQmlContext(m_engine.get()));
    m_engine->rootContext()->setContextProperty(QStringLiteral("app"), this);

    QQmlComponent component(m_engine.get(), u"org.kde.plasma.printqueue"_s, u"Main"_s);
    auto object = component.create(m_engine->rootContext());
    auto window = qobject_cast<QQuickWindow *>(object);
    if (!object || !window) {
        qCWarning(PMQUEUE) << "Failed to create main window:" << component.errorString();
        if (object) {
            delete object;
        }
        return;
    }
    m_mainWindow = std::unique_ptr<QQuickWindow>(window);

    // takes a quick minute for the models to populate for QML side
    QTimer::singleShot(200, this, [this, printerName]() {
        Q_EMIT showQueue(printerName);
    });
}

PrintQueue::~PrintQueue()
{
    m_engine->deleteLater();
}

QQuickWindow *PrintQueue::mainWindow() const
{
    return m_mainWindow.get();
}

void PrintQueue::init(const QString &printerName)
{
    m_mainWindow->show();
    Q_EMIT showQueue(printerName);
}

void PrintQueue::authenticateJob(const QString &printerName, int jobId, bool isClass)
{
    const auto authRequest = new KCupsRequest;
    connect(authRequest, &KCupsRequest::finished, this, [this, printerName, jobId](KCupsRequest *request) {
        request->deleteLater();

        if (request->hasError() || request->printers().size() != 1) {
            qCWarning(PMQUEUE) << "Ignoring request, printer not found or error" << printerName << request->errorMsg();
            return;
        }

        const auto printer = request->printers().at(0);
        KIO::AuthInfo info;
        info.keepPassword = true;
        info.prompt = i18n("Enter credentials to print from <b>%1</b>", printerName);
        info.url = QUrl(printer.uriSupported());
        if (printer.authInfoRequired().contains(QStringLiteral("domain"))) {
            info.setExtraField(QStringLiteral("domain"), QStringLiteral(""));
        }

        QScopedPointer<KPasswdServerClient> passwdServerClient(new KPasswdServerClient());
        auto winId = static_cast<qlonglong>(m_mainWindow->winId());
        auto usertime = static_cast<qlonglong>(KUserTimestamp::userTimestamp());
        if (passwdServerClient->checkAuthInfo(&info, winId, usertime)) {
            // at this stage we don't know if stored credentials are correct
            // trying blindly is risky, it may block this user's account
            passwdServerClient->removeAuthInfo(info.url.host(), info.url.scheme(), info.username);
        }
        const int passwordDialogErrorCode = passwdServerClient->queryAuthInfo(&info, QString(), winId, usertime);
        if (passwordDialogErrorCode != KJob::NoError) {
            // user cancelled or kiod_kpasswdserver not running
            qCDebug(PMQUEUE) << "queryAuthInfo error code" << passwordDialogErrorCode;
            return;
        }

        QStringList authInfo;
        for (QString &authInfoRequired : printer.authInfoRequired()) {
            if (authInfoRequired == QStringLiteral("domain")) {
                authInfo << info.getExtraField(QStringLiteral("domain")).toString();
            } else if (authInfoRequired == QStringLiteral("username")) {
                authInfo << info.username;
            } else if (authInfoRequired == QStringLiteral("password")) {
                authInfo << info.password;
            } else {
                qCWarning(PMQUEUE) << "No auth info for: " << authInfoRequired;
                authInfo << QString();
            }
        }

        const auto authRequest2 = new KCupsRequest;
        connect(authRequest2, &KCupsRequest::finished, this, [&passwdServerClient, info](KCupsRequest *req) {
            req->deleteLater();
            if (req->hasError()) {
                qCWarning(PMQUEUE) << "Error authenticating jobs" << req->error() << req->errorMsg();
                // remove cache on fail
                passwdServerClient->removeAuthInfo(info.url.host(), info.url.scheme(), info.username);
            }
        });
        authRequest2->authenticateJob(printerName, authInfo, jobId);
    });

    authRequest->getPrinterAttributes(printerName, isClass, {KCUPS_PRINTER_URI_SUPPORTED, KCUPS_AUTH_INFO_REQUIRED});
}

#include "moc_PrintQueue.cpp"
