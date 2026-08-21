/*
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CommandHelpers.h"
#include "kcupslib_log.h"
#include <KLocalizedString>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRegularExpression>
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

#if CUPS_VERSION_MAJOR < 3
    if (args.contains(u"ppd-type"_s)) {
        const auto ppdType = args.take(u"ppd-type"_s).toInt();
        if (static_cast<PPDType>(ppdType) == PPDType::Manual) {
            fileName = args.take(u"ppd-name"_s).toString();
        }
    }
#endif

    const bool addMode = args.take(u"add"_s).toBool();
    // Will only be set if default is changed to true
    const bool isDefault = args.take(u"isDefault"_s).toBool();

    if (addMode) {
        args[KCUPS_PRINTER_STATE] = IPP_PSTATE_IDLE;
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
                Q_EMIT error(KCupsCompat::kcupsError(),
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

DnssdParts PrinterCommands::parseUnresolvedUri(const QString &urlStr)
{
    DnssdParts parts;

    const auto rest = urlStr.section(u"://"_s, 1);
    int end = rest.indexOf(QRegularExpression(u"[/?]"_s));
    const auto encodedHost = (end == -1) ? rest : rest.left(end);
    const auto decodedHost = QUrl::fromPercentEncoding(encodedHost.toUtf8());

    static QRegularExpression re(R"(^(.*)\.(_[^.]+\._[^.]+)\.([^.]+)$)"_L1);
    auto m = re.match(decodedHost);
    if (m.hasMatch()) {
        parts.name = m.captured(1);
        parts.type = m.captured(2); // picks up "_ipps._tcp" automatically
        parts.domain = m.captured(3);
        parts.scheme = parts.type.contains(u"ipps"_s) ? u"ipps"_s : u"ipp"_s;
    }
    return parts;
}

QString PrinterCommands::resolveToUri(const QString &deviceUri)
{
    if (!deviceUri.startsWith(u"dnssd"_s) && !deviceUri.contains(u"._ipp")) {
        qCDebug(LIBKCUPS) << "DeviceUri does not need to be resolved:" << deviceUri;
        return {};
    }

    DnssdParts parts = parseUnresolvedUri(deviceUri);
    QDBusInterface avahi(u"org.freedesktop.Avahi"_s, u"/"_s, u"org.freedesktop.Avahi.Server"_s, QDBusConnection::systemBus());
    if (!avahi.isValid()) {
        qCWarning(LIBKCUPS) << "Avahi Resolver Service unavailable.  Some IPP-only features may be unavailable.";
        return {};
    }

    // ResolveService(interface, protocol, name, type, domain, aprotocol, flags)
    QDBusMessage reply = avahi.call(u"ResolveService"_s,
                                    -1,
                                    -1, // AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC
                                    parts.name,
                                    parts.type,
                                    parts.domain,
                                    -1, // AVAHI_PROTO_UNSPEC for resolved address
                                    0u // no flags
    );

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(LIBKCUPS) << "ResolveService failed:" << reply.errorMessage();
        return {};
    }
    // Reply args (in order): interface, protocol, name, type, domain,
    // host_name, aprotocol, address, port, txt, flags
    QList<QVariant> args = reply.arguments();
    qCDebug(LIBKCUPS) << "Avahi ResolveService return args:" << args;
    if (args.size() < 10) {
        return {};
    }

    const auto hostName = args[5].toString();
    const auto port = args[8].toString();

    // txt records come back as array of byte arrays "key=value"
    QString rp = "ipp/print"_L1;
    auto txtArg = args[9].value<QDBusArgument>();
    txtArg.beginArray();
    while (!txtArg.atEnd()) {
        QByteArray entry;
        txtArg >> entry;
        if (entry.startsWith("rp=")) {
            rp = QString::fromUtf8(entry.mid(3));
            break;
        }
    }
    txtArg.endArray();

    return QString(u"%1://%2:%3/%4"_s).arg(parts.scheme, hostName, port, rp);
}

#include "moc_CommandHelpers.cpp"
