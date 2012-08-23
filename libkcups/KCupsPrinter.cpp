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

#include "KCupsPrinter.h"

#include <KDebug>

KCupsPrinter::KCupsPrinter() :
    m_isClass(false)
{
}

KCupsPrinter::KCupsPrinter(const QString &printer, bool isClass) :
    m_printer(printer),
    m_isClass(isClass)
{
}

KCupsPrinter::KCupsPrinter(const QVariantHash &arguments) :
    m_arguments(arguments)
{
    m_printer = arguments[KCUPS_PRINTER_NAME].toString();
    m_isClass = arguments[KCUPS_PRINTER_TYPE].toInt() & CUPS_PRINTER_CLASS;
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
    return m_arguments[KCUPS_PRINTER_TYPE].toUInt() & CUPS_PRINTER_DEFAULT;
}

bool KCupsPrinter::isShared() const
{
    return m_arguments[KCUPS_PRINTER_IS_SHARED].toBool();
}

bool KCupsPrinter::isAcceptingJobs() const
{
    return m_arguments[KCUPS_PRINTER_IS_ACCEPTING_JOBS].toBool();
}

cups_ptype_e KCupsPrinter::type() const
{
    return static_cast<cups_ptype_e>(m_arguments[KCUPS_PRINTER_TYPE].toUInt());
}

QString KCupsPrinter::location() const
{
    return m_arguments[KCUPS_PRINTER_LOCATION].toString();
}

QString KCupsPrinter::info() const
{
    if (m_arguments[KCUPS_PRINTER_INFO].toString().isEmpty()) {
        return name();
    }
    return m_arguments[KCUPS_PRINTER_INFO].toString();
}

QString KCupsPrinter::makeAndModel() const
{
    return m_arguments[KCUPS_PRINTER_MAKE_AND_MODEL].toString();
}

QStringList KCupsPrinter::commands() const
{
    return m_arguments[KCUPS_PRINTER_COMMANDS].toStringList();
}

QStringList KCupsPrinter::memberNames() const
{
    return m_arguments[KCUPS_MEMBER_NAMES].toStringList();
}

QString KCupsPrinter::deviceUri() const
{
    return m_arguments[KCUPS_DEVICE_URI].toString();
}

QStringList KCupsPrinter::errorPolicy() const
{
    return m_arguments[KCUPS_PRINTER_ERROR_POLICY].toStringList();
}

QStringList KCupsPrinter::errorPolicySupported() const
{
    return m_arguments[KCUPS_PRINTER_ERROR_POLICY_SUPPORTED].toStringList();
}

QStringList KCupsPrinter::opPolicy() const
{
    return m_arguments[KCUPS_PRINTER_OP_POLICY].toStringList();
}

QStringList KCupsPrinter::opPolicySupported() const
{
    return m_arguments[KCUPS_PRINTER_OP_POLICY_SUPPORTED].toStringList();
}

QStringList KCupsPrinter::jobSheetsDefault() const
{
    return m_arguments[KCUPS_JOB_SHEETS_DEFAULT].toStringList();
}

QStringList KCupsPrinter::jobSheetsSupported() const
{
    return m_arguments[KCUPS_JOB_SHEETS_SUPPORTED].toStringList();
}

QStringList KCupsPrinter::requestingUserNameAllowed() const
{
    return m_arguments[KCUPS_REQUESTING_USER_NAME_ALLOWED].toStringList();
}

QStringList KCupsPrinter::requestingUserNameDenied() const
{
    return m_arguments[KCUPS_REQUESTING_USER_NAME_DENIED].toStringList();
}

QString KCupsPrinter::uriSupported() const
{
    return m_arguments[KCUPS_PRINTER_URI_SUPPORTED].toString();
}

KCupsPrinter::Status KCupsPrinter::state() const
{
    return static_cast<Status>(m_arguments[KCUPS_PRINTER_STATE].toUInt());
}

QString KCupsPrinter::stateMsg() const
{
    return m_arguments[KCUPS_PRINTER_STATE_MESSAGE].toString();
}

int KCupsPrinter::markerChangeTime() const
{
    return m_arguments[KCUPS_MARKER_CHANGE_TIME].toInt();
}

QVariant KCupsPrinter::argument(const QString &name) const
{
    return m_arguments.value(name);
}

KIcon KCupsPrinter::icon() const
{
    return icon(type());
}

KIcon KCupsPrinter::icon(cups_ptype_e type)
{
    return KIcon(iconName(type));
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
        return "printer-laser";
    } else if (type & CUPS_PRINTER_SCANNER) {
        return "scanner";
    } else {
        return "printer";
    }
}
