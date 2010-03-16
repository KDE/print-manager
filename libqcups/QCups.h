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
        Q_PROPERTY(QString description READ description WRITE setDescription USER true)
        Q_PROPERTY(QString location READ location WRITE setLocation)
        Q_PROPERTY(QString makeAndModel READ makeAndModel WRITE setMakeAndModel)
        Q_PROPERTY(bool shared READ shared WRITE setShared)
    public:
        Printer(QObject *parent = 0);
        Printer(const QString &destName, QObject *parent = 0);

        void setDescription(const QString &description);
        void setLocation(const QString &location);
        void setMakeAndModel(const QString &makeAndModel);
        void setShared(bool shared);
        QString description() const;
        QString location() const;
        QString makeAndModel() const;
        bool shared() const;

        bool save();

        static bool setShared(const QString &destName, bool shared);

    private:
        QString m_destName;
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
    KDE_EXPORT bool addModifyPrinter(const QString &name, const QHash<QString, QVariant> values);
};

#endif
