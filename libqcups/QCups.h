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
#include <QObject>
#include <QHash>

#define DEST_IDLE     '3'
#define DEST_PRINTING '4'
#define DEST_STOPED   '5'

namespace QCups
{
    class KDE_EXPORT Printer: public QObject
    {
        Q_OBJECT
    public:
        Printer(QObject *parent = 0);
        explicit Printer(const QString &destName, QObject *parent = 0);

        QString value(const QString &name) const;

        bool setAttributes(bool isClass, const QHash<QString, QVariant> &values);
        static bool setAttributes(const QString &destName, bool isClass, const QHash<QString, QVariant> &values);

        static bool setShared(const QString &destName, bool isClass, bool shared);
        static QHash<QString, QVariant> getAttributes(const QString &destName, bool isClass, const QStringList &requestedAttr);
        static bool setAttributesFile(const QString &destName, const QStringList &requestedAttr);

    private:
        QString m_destName;
        QHash<QString, QString> m_values;
    };

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

    KDE_EXPORT QList<QPair<QString, QString> > getDests(int mask);
};

#endif
