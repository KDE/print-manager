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

KCupsPrinter::KCupsPrinter() :
    m_isClass(false)
{
}

KCupsPrinter::KCupsPrinter(const QString &printer, bool isClass) :
    m_printer(printer),
    m_isClass(isClass)
{
}

KCupsPrinter::KCupsPrinter(const Arguments &arguments) :
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

QString KCupsPrinter::description() const
{
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

int KCupsPrinter::state() const
{
    return m_arguments["printer-state"].toInt();
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
    // TODO get the ppd or something to get the real printer icon
//    Q_UNUSED(printer)

    if (!(type() & CUPS_PRINTER_COLOR)) {
        // If the printer is not color it is probably a laser one
        return KIcon("printer-laser");
    } else if (type() & CUPS_PRINTER_SCANNER) {
        return KIcon("scanner");
    } else {
        return KIcon("printer");
    }
}

