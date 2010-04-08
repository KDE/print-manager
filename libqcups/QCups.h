/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#ifndef Q_CUPS_H
#define Q_CUPS_H

#include <kdemacros.h>
#include <QStringList>
#include <QHash>

#define DEST_IDLE     3
#define DEST_PRINTING 4
#define DEST_STOPED   5

namespace QCups
{
    // Dest Methods
    namespace Dest
    {
        KDE_EXPORT bool setAttributes(const QString &destName, bool isClass, const QHash<QString, QVariant> &values, const char *filename = NULL);

        KDE_EXPORT bool setShared(const QString &destName, bool isClass, bool shared);
        KDE_EXPORT QHash<QString, QVariant> getAttributes(const QString &destName, bool isClass, const QStringList &requestedAttr);
        KDE_EXPORT bool printTestPage(const QString &destName, bool isClass);
        KDE_EXPORT bool printCommand(const QString &destName, const QString &command, const QString &title);
    }

    KDE_EXPORT void initialize();
    KDE_EXPORT bool cancelJob(const QString &name, int job_id);
    KDE_EXPORT bool holdJob(const QString &name, int job_id);
    KDE_EXPORT bool releaseJob(const QString &name, int job_id);
    KDE_EXPORT bool moveJob(const QString &name, int job_id, const QString &dest_name);
    KDE_EXPORT bool pausePrinter(const QString &name);
    KDE_EXPORT bool resumePrinter(const QString &name);
    KDE_EXPORT bool setDefaultPrinter(const QString &name);
    KDE_EXPORT bool deletePrinter(const QString &name);
    KDE_EXPORT bool addModifyClassOrPrinter(const QString &name, bool isClass, const QHash<QString, QVariant> values);
    KDE_EXPORT bool adminSetServerSettings(const QHash<QString, QString> &userValues);

    KDE_EXPORT QList<QHash<QString, QVariant> > getPPDS(const QString &make = QString());

    typedef QHash<QString, QVariant> Destination;
    // THIS function can get the default server dest throught
    // "printer-is-default" attribute BUT it does not get user
    // defined default printer, see cupsGetDefault() on www.cups.org for details
    KDE_EXPORT QList<Destination> getDests(int mask, const QStringList &requestedAttr = QStringList());
    KDE_EXPORT QHash<QString, QString> adminGetServerSettings();
};

#endif
