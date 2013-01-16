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

#ifndef KCUPSJOB_H
#define KCUPSJOB_H

#include <QString>
#include <QDateTime>

#include <KCupsConnection.h>

class KDE_EXPORT KCupsJob
{
    Q_GADGET
    Q_ENUMS(Attribute)
public:
    KCupsJob();
    KCupsJob(int jobId, const QString &printer);

    int id() const;
    QString idStr() const;
    QString name() const;
    QString ownerName() const;
    QString printer() const;
    QDateTime createdAt() const;
    QDateTime completedAt() const;
    QDateTime processedAt() const;
    int pages() const;
    int processedPages() const;
    int size() const;

    static QString iconName(ipp_jstate_t state);
    ipp_jstate_t state() const;
    QString stateMsg() const;

    static bool cancelEnabled(ipp_jstate_t state);
    static bool holdEnabled(ipp_jstate_t state);
    static bool releaseEnabled(ipp_jstate_t state);
    static bool restartEnabled(ipp_jstate_t state);

protected:
    KCupsJob(const QVariantHash &arguments);

private:
    friend class KCupsRequest;

    int     m_jobId;
    QString m_printer;
    QVariantHash m_arguments;
};

typedef QList<KCupsJob> KCupsJobs;
Q_DECLARE_METATYPE(KCupsJobs)
Q_DECLARE_METATYPE(KCupsJob)

#endif // KCUPSJOB_H
