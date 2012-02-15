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
#include <KCupsConnection.h>

class KDE_EXPORT KCupsPrinter
{
public:
    typedef enum {
        Idle = 3,
        Printing,
        Stoped
    } Status;
    KCupsPrinter();
    KCupsPrinter(const QString &printer, bool isClass = false);

    QString name() const;
    bool isClass() const;
    bool isDefault() const;
    bool isShared() const;
    cups_ptype_e type() const;
    QString location() const;
    QString description() const;
    QString makeAndModel() const;
    QStringList commands() const;

    Status state() const;
    QString stateMsg() const;
    int markerChangeTime() const;
    QVariant argument(const QString &name) const;
    static KIcon icon(cups_ptype_e type);
    KIcon icon() const;

protected:
    KCupsPrinter(const QVariantHash &arguments);

private:
    friend class KCupsRequest;

    QString m_printer;
    bool    m_isClass;
    QVariantHash m_arguments;
};

#endif // KCUPSPRINTER_H
