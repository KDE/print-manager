/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcupslib_log.h"

#include "KCupsPrinter.h"
#include <KLocalizedString>

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

QStringList KCupsPrinter::checkMarkerLevels() const
{
    const auto currentLevels = markerLevels();
    const auto lowLevels = markerLowLevels();
    const auto highLevels = markerHighLevels();
    const auto typesList = markerTypes();
    const auto namesList = markerNames();

    // All lists should have values for a valid marker check
    if (currentLevels.isEmpty() || highLevels.isEmpty() || lowLevels.isEmpty() || typesList.isEmpty() || namesList.isEmpty()) {
        qCWarning(LIBKCUPS) << "At least one marker level attribute is empty, aborting level check" << currentLevels << lowLevels << highLevels << typesList
                            << namesList;
        return {};
    }

    // We address all lists by index so they had better be of uniform length.
    const auto levelCounts = {currentLevels.count(), lowLevels.count(), highLevels.count(), typesList.count(), namesList.count()};
    const auto levelsMaxCount = std::max(levelCounts);
    if (std::ranges::any_of(levelCounts, [levelsMaxCount](auto count) {
            return count != levelsMaxCount;
        })) {
        QString message;
        QDebug(&message) << "Marker level lists have different counts, aborting level check" << currentLevels << lowLevels << highLevels << typesList
                         << namesList;
        qCWarning(LIBKCUPS) << message;
        // This is a fairly serious problem we should inspect. Fail an assertion on it!
        // Can be safely demoted when we have some more confidence that we understand why the check may fail.
        Q_ASSERT_X(false, Q_FUNC_INFO, message.toUtf8().constData());
        return {};
    }

    QStringList msgs;
    constexpr int empty = 0;
    constexpr int nearEmpty = 3;
    constexpr int full = 100;
    // For each marker
    for (uint i = 0; i < namesList.count(); ++i) {
        // Per OpenPrinting, level < 0 indicates unknown or unavailable
        const auto level = currentLevels.at(i);
        if (level < empty) {
            qCDebug(LIBKCUPS, "Invalid level (%d), ignore checking: %s, %s", level, qPrintable(typesList.at(i)), qPrintable(namesList.at(i)));
            continue;
        }

        int levelIndex = -1;
        int checkerValue = 0;

        auto low = lowLevels.at(i);
        auto high = highLevels.at(i);

        /**
         * Per OpenPrinting
         * marker-low-levels = 0 AND marker-high-levels < 100 -> receptacle
         * "near full" warning at marker-high-levels value
         *
         * marker-low-levels > 0 AND marker-high-levels = 100 -> consumable
         * "near empty"/"low" warning at marker-low-levels value
         *
         * marker-low-levels = 0 AND marker-high-levels = 100 is a firmware bug
         * assume marker-low-levels = 3 (nearEmpty) which turns it into a consumable
         * and a "near empty"/"low" warning is issued when 3% of it is left.
         *
         * marker-low-levels > 0 AND marker-high-levels < 100 is also a firmware bug
         * assume marker-high-levels = 100 which turns it into a consumable and a
         * "near empty"/"low" warning is issued when its level reaches the marker-low-levels value
         *
         * This assures that in case of correct firmware with correct supply-level reporting
         * everything is done exactly as the printer manufacturer expects it and in case of
         * buggy firmware which reports supply level info which does not allow to determine
         * whether a supply is a consumable or a receptacle we assume that it is a consumable
         * as most supplies are consumables.
         */
        bool consumable = false;

        // probable firmware bugs
        if (low == empty && high == full) {
            // default low, set as a consumable
            qCDebug(LIBKCUPS, "%s: possible firmware bug (%d,%d): setting low-level to: %d", qPrintable(namesList.at(i)), low, high, nearEmpty);
            low = nearEmpty;
        } else if (low > empty && high < full) {
            // default high, set as a consumable
            qCDebug(LIBKCUPS, "%s: possible firmware bug (%d,%d): setting high-level to: %d", qPrintable(namesList.at(i)), low, high, full);
            high = full;
        }

        // first, check for receptacle
        if (low == empty && high < full) {
            // receptacle
            consumable = false;
            qCDebug(LIBKCUPS, "Found receptacle: checking %s, current level %d => (%d,%d)", qPrintable(namesList.at(i)), level, low, high);
            if (level >= high) {
                levelIndex = i;
                checkerValue = level;
            }
        } else if (low > empty && high == full) {
            // consumable
            consumable = true;
            qCDebug(LIBKCUPS, "Found consumable: checking %s, current level %d => (%d,%d)", qPrintable(namesList.at(i)), level, low, high);
            if (level <= low) {
                levelIndex = i;
                checkerValue = level;
            }
        } else {
            qCDebug(LIBKCUPS, "low/high range invalid (%d,%d): failure to determine marker type for: %s", low, high, qPrintable(typesList.at(i)));
            continue;
        }

        // found a marker level at or below the warning boundary
        if (levelIndex >= 0) {
            const auto name = namesList.at(levelIndex);
            const auto type = typesList.at(levelIndex);

            if (consumable) {
                if (checkerValue == 0) {
                    msgs << i18nc("@info:usagetip The name and type of the ink cartridge", "%1 (%2) appears to be empty.", name, type);
                } else {
                    msgs << i18nc("@info:usagetip The name and type of the ink cartridge and percent ink remaining",
                                  "%1 (%2) appears to be low (%3% remaining).",
                                  name,
                                  type,
                                  checkerValue);
                }
            } else {
                msgs << i18nc("@info:usagetip The name, type, and percentage full of the waste receptacle",
                              "%1 (%2) is almost full (%3%)",
                              name,
                              type,
                              checkerValue);
            }
            qCDebug(LIBKCUPS, "Send level notify on: %s, %s, level=%d", qPrintable(type), qPrintable(name), checkerValue);
        }
    }

    return msgs;
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
    if (val.isValid()) {
        if (val.canConvert<QList<int>>()) {
            return val.value<QList<int>>();
        } else {
            return QList<int>{val.value<int>()};
        }
    }
    return {};
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
