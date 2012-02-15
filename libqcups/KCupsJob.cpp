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

#include "KCupsJob.h"

#include <KDebug>

KCupsJob::KCupsJob() :
    m_jobId(0)
{
}

KCupsJob::KCupsJob(int jobId, const QString &printer) :
    m_jobId(jobId),
    m_printer(printer)
{
}

KCupsJob::KCupsJob(const QVariantHash &arguments) :
    m_arguments(arguments)
{
    m_jobId = arguments["job-id"].toInt();
//    m_printer = arguments["dest-name"].toString();
    m_printer = arguments["job-printer-uri"].toString().section('/', -1);
}

int KCupsJob::id() const
{
    return m_jobId;
}

QString KCupsJob::name() const
{
    return m_arguments["job-name"].toString();
}

QString KCupsJob::ownerName() const
{
    return m_arguments["job-originating-user-name"].toString();
}

QString KCupsJob::printer() const
{
    return m_printer;
}

QDateTime KCupsJob::createdAt() const
{
    QDateTime ret;
    ret.setTime_t(m_arguments["time-at-creation"].toInt());
    return ret;
}

QDateTime KCupsJob::completedAt() const
{
    QDateTime ret;
    ret.setTime_t(m_arguments["time-at-completed"].toInt());
    return ret;
}

QDateTime KCupsJob::processedAt() const
{
    QDateTime ret;
    ret.setTime_t(m_arguments["time-at-processing"].toInt());
    return ret;
}

int KCupsJob::completedPages() const
{
    return m_arguments["job-media-sheets-completed"].toInt();
}

int KCupsJob::size() const
{
    int jobKOctets = m_arguments["job-k-octets"].toInt();
    jobKOctets *= 1024; // transform it to bytes
    return jobKOctets;
}

ipp_jstate_e KCupsJob::state() const
{
    return static_cast<ipp_jstate_e>(m_arguments["job-state"].toUInt());
}

QString KCupsJob::stateMsg() const
{
    return m_arguments["job-printer-state-message"].toString();
}
