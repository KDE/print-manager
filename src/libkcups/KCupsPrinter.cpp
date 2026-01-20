/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsPrinter.h"

KCupsPrinter::KCupsPrinter()
    : m_isClass(false)
{
}

KCupsPrinter::KCupsPrinter(const QString &printer, bool isClass)
    : m_printer(printer)
    , m_isClass(isClass)
{
}

KCupsPrinter::KCupsPrinter(const QVariantMap &attributes)
    : m_printer(attributes[KCUPS_PRINTER_NAME].toString())
    , m_isClass(attributes[KCUPS_PRINTER_TYPE].toInt() & CUPS_PRINTER_CLASS)
    , m_attributes(attributes)
{
}

void KCupsPrinter::setAttribute(const QString &key, const QVariant &value)
{
    if (!key.isEmpty()) {
        m_attributes[key] = value;
    }
}

QString KCupsPrinter::name() const
{
    return m_printer;
}

bool KCupsPrinter::isClass() const
{
    return m_isClass;
}

bool KCupsPrinter::isDefault() const
{
    return m_attributes[KCUPS_PRINTER_TYPE].toUInt() & CUPS_PRINTER_DEFAULT;
}

bool KCupsPrinter::isShared() const
{
    return m_attributes[KCUPS_PRINTER_IS_SHARED].toBool();
}

bool KCupsPrinter::isAcceptingJobs() const
{
    return m_attributes[KCUPS_PRINTER_IS_ACCEPTING_JOBS].toBool();
}

cups_ptype_e KCupsPrinter::type() const
{
    return static_cast<cups_ptype_e>(m_attributes[KCUPS_PRINTER_TYPE].toUInt());
}

QString KCupsPrinter::location() const
{
    return m_attributes[KCUPS_PRINTER_LOCATION].toString();
}

QString KCupsPrinter::info() const
{
    const QString printerInfo = m_attributes[KCUPS_PRINTER_INFO].toString();
    if (printerInfo.isEmpty()) {
        return name();
    }
    return printerInfo;
}

QString KCupsPrinter::makeAndModel() const
{
    return m_attributes[KCUPS_PRINTER_MAKE_AND_MODEL].toString();
}

QStringList KCupsPrinter::commands() const
{
    return m_attributes[KCUPS_PRINTER_COMMANDS].toStringList();
}

QStringList KCupsPrinter::memberNames() const
{
    return m_attributes[KCUPS_MEMBER_NAMES].toStringList();
}

QString KCupsPrinter::deviceUri() const
{
    return m_attributes[KCUPS_DEVICE_URI].toString();
}

QStringList KCupsPrinter::errorPolicy() const
{
    return m_attributes[KCUPS_PRINTER_ERROR_POLICY].toStringList();
}

QStringList KCupsPrinter::errorPolicySupported() const
{
    return m_attributes[KCUPS_PRINTER_ERROR_POLICY_SUPPORTED].toStringList();
}

QStringList KCupsPrinter::opPolicy() const
{
    return m_attributes[KCUPS_PRINTER_OP_POLICY].toStringList();
}

QStringList KCupsPrinter::opPolicySupported() const
{
    return m_attributes[KCUPS_PRINTER_OP_POLICY_SUPPORTED].toStringList();
}

QStringList KCupsPrinter::jobSheetsDefault() const
{
    return m_attributes[KCUPS_JOB_SHEETS_DEFAULT].toStringList();
}

QStringList KCupsPrinter::jobSheetsSupported() const
{
    return m_attributes[KCUPS_JOB_SHEETS_SUPPORTED].toStringList();
}

QStringList KCupsPrinter::requestingUserNameAllowed() const
{
    return m_attributes[KCUPS_REQUESTING_USER_NAME_ALLOWED].toStringList();
}

QStringList KCupsPrinter::requestingUserNameDenied() const
{
    return m_attributes[KCUPS_REQUESTING_USER_NAME_DENIED].toStringList();
}

QStringList KCupsPrinter::authInfoRequired() const
{
    return m_attributes[KCUPS_AUTH_INFO_REQUIRED].toStringList();
}

QString KCupsPrinter::uriSupported() const
{
    return m_attributes[KCUPS_PRINTER_URI_SUPPORTED].toString();
}

KCupsPrinter::Status KCupsPrinter::state() const
{
    return static_cast<Status>(m_attributes[KCUPS_PRINTER_STATE].toUInt());
}

QString KCupsPrinter::stateMsg() const
{
    return m_attributes[KCUPS_PRINTER_STATE_MESSAGE].toString();
}

int KCupsPrinter::markerChangeTime() const
{
    return m_attributes[KCUPS_MARKER_CHANGE_TIME].toInt();
}

QStringList KCupsPrinter::markerColors() const
{
    return m_attributes[KCUPS_MARKER_COLORS].toStringList();
}

QStringList KCupsPrinter::markerNames() const
{
    return m_attributes[KCUPS_MARKER_NAMES].toStringList();
}

QStringList KCupsPrinter::markerTypes() const
{
    return m_attributes[KCUPS_MARKER_TYPES].toStringList();
}

static QList<int> toIntList(const QVariant &val)
{
    if (val.canConvert<QList<int>>()) {
        return val.value<QList<int>>();
    } else {
        return {};
    }
}

QList<int> KCupsPrinter::markerLowLevels() const
{
    return toIntList(m_attributes.value(KCUPS_MARKER_LOW_LEVELS));
}

QList<int> KCupsPrinter::markerHighLevels() const
{
    return toIntList(m_attributes.value(KCUPS_MARKER_HIGH_LEVELS));
}

QList<int> KCupsPrinter::markerLevels() const
{
    return toIntList(m_attributes.value(KCUPS_MARKER_LEVELS));
}

QVariantMap KCupsPrinter::markers() const
{
    const auto levels = markerLevels();
    if (levels.isEmpty()) {
        return {};
    } else {
        return {{KCUPS_MARKER_LEVELS, QVariant::fromValue(levels)},
                {KCUPS_MARKER_COLORS, markerColors()},
                {KCUPS_MARKER_NAMES, markerNames()},
                {KCUPS_MARKER_TYPES, markerTypes()}};
    }
}

QVariant KCupsPrinter::argument(const QString &name) const
{
    return m_attributes.value(name);
}

QIcon KCupsPrinter::icon() const
{
    return icon(type());
}

QIcon KCupsPrinter::icon(cups_ptype_e type)
{
    return QIcon::fromTheme(iconName(type));
}

QString KCupsPrinter::iconName() const
{
    return iconName(type());
}

QString KCupsPrinter::iconName(cups_ptype_e type)
{
    // TODO get the ppd or something to get the real printer icon
    if (!(type & CUPS_PRINTER_COLOR)) {
        // If the printer is not color it is probably a laser one
        return QStringLiteral("printer-laser");
    } else if (type & CUPS_PRINTER_SCANNER) {
        return QStringLiteral("scanner");
    } else {
        return QStringLiteral("printer");
    }
}
