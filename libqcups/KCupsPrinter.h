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

#ifndef KCUPSPRINTER_H
#define KCUPSPRINTER_H

#include <QString>

#include <KIcon>

#include <KCupsConnection.h>

class KDE_EXPORT KCupsPrinter
{
    Q_GADGET
    Q_ENUMS(Attribute)
public:
    enum Attribute {
        PrinterName                 = 1 << 0,
        PrinterState                = 1 << 1,
        PrinterStateMessage         = 1 << 2,
        PrinterIsShared             = 1 << 3,
        PrinterType                 = 1 << 4,
        PrinterLocation             = 1 << 5,
        PrinterInfo                 = 1 << 6,
        PrinterMakeAndModel         = 1 << 7,
        PrinterCommands             = 1 << 8,
        PrinterUriSupported         = 1 << 9,
        MarkerChangeTime            = 1 << 10,
        MarkerColors                = 1 << 11,
        MarkerLevels                = 1 << 12,
        MarkerHighLevels            = 1 << 13,
        MarkerLowLevels             = 1 << 14,
        MarkerNames                 = 1 << 15,
        MarkerMessage               = 1 << 16,
        MarkerTypes                 = 1 << 17,
        MemberNames                 = 1 << 18,
        DeviceUri                   = 1 << 19,
        JobSheetsDefault            = 1 << 20,
        JobSheetsSupported          = 1 << 21,
        PrinterErrorPolicy          = 1 << 22,
        PrinterErrorPolicySupported = 1 << 23,
        PrinterOpPolicy             = 1 << 24,
        PrinterOpPolicySupported    = 1 << 25,
        RequestingUserNameAllowed   = 1 << 26,
        RequestingUserNameDenied    = 1 << 27
    };
    Q_DECLARE_FLAGS(Attributes, Attribute)
    typedef enum {
        Idle = 3,
        Printing,
        Stoped
    } Status;
    KCupsPrinter();
    explicit KCupsPrinter(const QString &printer, bool isClass = false);

    static QStringList flags(const Attributes &attributes);

    QString name() const;
    bool isClass() const;
    bool isDefault() const;
    bool isShared() const;
    cups_ptype_e type() const;
    QString location() const;
    QString info() const;
    QString makeAndModel() const;
    QStringList commands() const;
    QStringList memberNames() const;
    QString deviceUri() const;
    QStringList errorPolicy() const;
    QStringList errorPolicySupported() const;
    QStringList opPolicy() const;
    QStringList opPolicySupported() const;
    QStringList jobSheetsDefault() const;
    QStringList jobSheetsSupported() const;
    QStringList requestingUserNameAllowed() const;
    QStringList requestingUserNameDenied() const;
    QString uriSupported() const;

    Status state() const;
    QString stateMsg() const;
    int markerChangeTime() const;
    QVariant argument(const QString &name) const;

    /**
      * Requires enum PrinterType to work properly
      *
      */
    KIcon icon() const;
    static KIcon icon(cups_ptype_e type);
    QString iconName() const;
    static QString iconName(cups_ptype_e type);

protected:
    KCupsPrinter(const QVariantHash &arguments);

private:
    friend class KCupsRequest;

    QString m_printer;
    bool    m_isClass;
    QVariantHash m_arguments;
};

Q_DECLARE_METATYPE(KCupsPrinter::Attributes)

#endif // KCUPSPRINTER_H
