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
#include <KCupsConnection.h>

class KDE_EXPORT KCupsJob
{
public:
    KCupsJob(int jobId, const QString &printer);

    int id() const;
    QString name() const;
    QString ownerName() const;
    QString printer() const;
    QDateTime createdAt() const;
    QDateTime completedAt() const;
    QDateTime processedAt() const;
    int completedPages() const;
    int size() const;

    ipp_jstate_e state() const;
    QString stateMsg() const;
    int markerChangeTime() const;

protected:
    KCupsJob(const Arguments &arguments);

private:
    friend class KCupsRequestServer;

    int     m_jobId;
    QString m_printer;
    Arguments m_arguments;
};

#endif // KCUPSJOB_H
