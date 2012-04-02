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
    m_printer = arguments["printer-name"].toString();
    m_isClass = arguments["printer-type"].toInt() & CUPS_PRINTER_CLASS;
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
    return m_arguments["printer-type"].toUInt() & CUPS_PRINTER_DEFAULT;
}

bool KCupsPrinter::isShared() const
{
    return m_arguments["printer-is-shared"].toBool();
}

cups_ptype_e KCupsPrinter::type() const
{
    return static_cast<cups_ptype_e>(m_arguments["printer-type"].toUInt());
}

QString KCupsPrinter::location() const
{
    return m_arguments["printer-location"].toString();
}

QString KCupsPrinter::info() const
{
    if (m_arguments["printer-info"].toString().isEmpty()) {
        return name();
    }
    return m_arguments["printer-info"].toString();
}

QString KCupsPrinter::makeAndModel() const
{
    return m_arguments["printer-make-and-model"].toString();
}

QStringList KCupsPrinter::commands() const
{
    return m_arguments["printer-commands"].toStringList();
}

QStringList KCupsPrinter::memberNames() const
{
    return m_arguments["member-names"].toStringList();
}

QString KCupsPrinter::deviceUri() const
{
    return m_arguments["device-uri"].toString();
}

QStringList KCupsPrinter::errorPolicy() const
{
    return m_arguments["printer-error-policy"].toStringList();
}

QStringList KCupsPrinter::errorPolicySupported() const
{
    return m_arguments["printer-error-policy-supported"].toStringList();
}

QStringList KCupsPrinter::opPolicy() const
{
    return m_arguments["printer-op-policy"].toStringList();
}

QStringList KCupsPrinter::opPolicySupported() const
{
    return m_arguments["printer-op-policy-supported"].toStringList();
}

QStringList KCupsPrinter::jobSheetsDefault() const
{
    return m_arguments["job-sheets-default"].toStringList();
}

QStringList KCupsPrinter::jobSheetsSupported() const
{
    return m_arguments["job-sheets-supported"].toStringList();
}

QStringList KCupsPrinter::requestingUserNameAllowed() const
{
    return m_arguments["requesting-user-name-allowed"].toStringList();
}

QStringList KCupsPrinter::requestingUserNameDenied() const
{
    return m_arguments["requesting-user-name-denied"].toStringList();
}

KCupsPrinter::Status KCupsPrinter::state() const
{
    return static_cast<Status>(m_arguments["printer-state"].toUInt());
}

QString KCupsPrinter::stateMsg() const
{
    return m_arguments["printer-state-message"].toString();
}

int KCupsPrinter::markerChangeTime() const
{
    return m_arguments["marker-change-time"].toInt();
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

QStringList KCupsPrinter::flags(const Attributes &attributes)
{
    QStringList ret;
    if (attributes & PrinterName) {
        ret << "printer-name";
    }
    if (attributes & PrinterState) {
        ret << "printer-state";
    }
    if (attributes & PrinterStateMessage) {
        ret << "printer-state-message";
    }
    if (attributes & PrinterIsShared) {
        ret << "printer-is-shared";
    }
    if (attributes & PrinterType) {
        ret << "printer-type";
    }
    if (attributes & PrinterLocation) {
        ret << "printer-location";
    }
    if (attributes & PrinterInfo) {
        ret << "printer-info";
    }
    if (attributes & PrinterMakeAndModel) {
        ret << "printer-make-and-model";
    }
    if (attributes & PrinterCommands) {
        ret << "printer-commands";
    }
    if (attributes & PrinterUriSupported) {
        ret << "printer-uri-supported";
    }
    if (attributes & MarkerChangeTime) {
        ret << "marker-change-time";
    }
    if (attributes & MarkerColors) {
        ret << "marker-colors";
    }
    if (attributes & MarkerLevels) {
        ret << "marker-levels";
    }
    if (attributes & MarkerHighLevels) {
        ret << "marker-high-levels";
    }
    if (attributes & MarkerLowLevels) {
        ret << "marker-low-levels";
    }
    if (attributes & MarkerNames) {
        ret << "marker-names";
    }
    if (attributes & MarkerTypes) {
        ret << "marker-types";
    }
    if (attributes & MarkerMessage) {
        ret << "marker-message";
    }
    if (attributes & MemberNames) {
        ret << "member-names";
    }
    if (attributes & DeviceUri) {
        ret << "device-uri";
    }
    if (attributes & JobSheetsDefault) {
        ret << "job-sheets-default";
    }
    if (attributes & JobSheetsSupported) {
        ret << "job-sheets-supported";
    }
    if (attributes & PrinterErrorPolicy) {
        ret << "printer-error-policy";
    }
    if (attributes & PrinterErrorPolicySupported) {
        ret << "printer-error-policy-supported";
    }
    if (attributes & PrinterOpPolicy) {
        ret << "printer-op-policy";
    }
    if (attributes & PrinterOpPolicySupported) {
        ret << "printer-op-policy-supported";
    }
    if (attributes & RequestingUserNameAllowed) {
        ret << "requesting-user-name-allowed";
    }
    if (attributes & RequestingUserNameDenied) {
        ret << "requesting-user-name-denied";
    }

    return ret;
}
