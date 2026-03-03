/*
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CommandHelpers.h"
#include "kcupslib_log.h"
#include <KLocalizedString>
#include <QVersionNumber>

using namespace Qt::StringLiterals;

PrinterCommands::PrinterCommands(QObject *parent)
    : QObject(parent)
{
}

PrinterCommands::~PrinterCommands()
{
}

void PrinterCommands::savePrinter(const QString &name, const QVariantMap &saveArgs, bool isClass)
{
    QVariantMap args = saveArgs;
    QString fileName;

    if (args.contains(u"ppd-type"_s)) {
        const auto ppdType = args.take(u"ppd-type"_s).toInt();
        if (static_cast<PPDType>(ppdType) == PPDType::Manual) {
            fileName = args.take(u"ppd-name"_s).toString();
        }
    }

    const bool addMode = args.take(u"add"_s).toBool();
    // Will only be set if default is changed to true
    const bool isDefault = args.take(u"isDefault"_s).toBool();

    if (addMode) {
        args[KCUPS_PRINTER_STATE] = IPP_PRINTER_IDLE;
    }

    // WORKAROUND: Remove after CUPS 2.4.13 release
    // CUPS Issue #1235 (https://github.com/OpenPrinting/cups/issues/1235)
    // Fixed in 2.4.13+/2.5 (N/A in CUPS 3.x)
    const bool forceRefresh = !addMode && (args.value(u"ppd-name"_s) == u"everywhere"_s)
        && (QVersionNumber(CUPS_VERSION_MAJOR, CUPS_VERSION_MINOR, CUPS_VERSION_PATCH) < QVersionNumber(2, 4, 13));

    qCDebug(LIBKCUPS) << (addMode ? "New Printer:" : "Change Printer:") << name
                      << "isClass?" << isClass
                      << "Changing Default?" << isDefault
                      << "filename" << fileName
                      << "forceRefresh" << forceRefresh
                      << "args" << args;

    if (isClass) {
        // Member list is a QVariantList, kcupslib wants to see a QStringList
        const auto list = args.take(KCUPS_MEMBER_URIS);
        if (!list.value<QVariantList>().empty()) {
            args.insert(KCUPS_MEMBER_URIS, list.toStringList());
        }
    }

    const auto checkDefault = [this, name, isDefault, forceRefresh]() {
        if (isDefault) {
            qCDebug(LIBKCUPS) << "Saving printer DEFAULT:" << name;
            connect(
                this,
                &PrinterCommands::defaultDone,
                this,
                [this, forceRefresh]() {
                    Q_EMIT saveDone(forceRefresh);
                },
                Qt::SingleShotConnection);
            setDefault(name);
        } else {
            Q_EMIT saveDone(forceRefresh);
        }
    };

    /** If no other printer attrs are changed, we still have to check default printer
     * Default printer is handled by CUPS independently of the other printer
     * attributes. if Default is set save explicitly.
     */
    if (args.isEmpty() && fileName.isEmpty()) {
        checkDefault();
    } else {
        const auto request = setupRequest(
            [checkDefault](KCupsRequest *) {
                checkDefault();
            },
            [this, isClass, name](KCupsRequest *req) {
                Q_EMIT error(cupsLastError(),
                             (isClass ? i18nc("@info", "Failed to configure class: ") : i18nc("@info", "Failed to configure printer: ")),
                             req->errorMsg());
                qCWarning(LIBKCUPS) << "Failed to save printer/class" << name << req->errorMsg();
            });

        if (isClass) {
            request->addOrModifyClass(name, args);
        } else {
            request->addOrModifyPrinter(name, args, fileName);
        }
    }
}

void PrinterCommands::setDefault(const QString &printerName)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT defaultDone(); });
    request->setDefaultPrinter(printerName);
}

void PrinterCommands::setShared(const QString &printerName, bool isClass, bool shared)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT sharedDone(); });
    request->setShared(printerName, isClass, shared);
}

void PrinterCommands::setAcceptingJobs(const QString &printerName, bool accept)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT acceptDone(); });
    if (accept) {
        request->acceptJobs(printerName);
    } else {
        request->rejectJobs(printerName);
    }
}

void PrinterCommands::printTestPage(const QString &printerName, bool isClass)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT testDone(); });
    request->printTestPage(printerName, isClass);
}

void PrinterCommands::printSelfTestPage(const QString &printerName)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT testDone(); });
    request->printCommand(printerName, u"PrintSelfTestPage"_s, i18n("Print Self-Test Page"));
}

void PrinterCommands::cleanPrintHeads(const QString &printerName)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT cleanDone(); });
    request->printCommand(printerName, u"Clean all"_s, i18n("Clean Print Heads"));
}

void PrinterCommands::pausePrinter(const QString &printerName)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT pauseDone(); });
    request->pausePrinter(printerName);
}

void PrinterCommands::removePrinter(const QString &printerName)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT removeDone(); });
    request->deletePrinter(printerName);
}

void PrinterCommands::resumePrinter(const QString &printerName)
{
    const auto request = setupRequest([this](KCupsRequest*) { Q_EMIT resumeDone(); });
    request->resumePrinter(printerName);
}

KCupsRequest *PrinterCommands::setupRequest(StdRequestCB success_cb, StdRequestCB error_cb)
{
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, [this, success_cb, error_cb](KCupsRequest *r) {
        if (r->hasError()) {
            if (error_cb) {
                error_cb(r);
            } else {
                Q_EMIT error(r->error(), r->serverError(), r->errorMsg());
            }
        } else {
            if (success_cb) {
                success_cb(r);
            }
        }
        r->deleteLater();
    });

    return request;
}

#include "moc_CommandHelpers.cpp"
