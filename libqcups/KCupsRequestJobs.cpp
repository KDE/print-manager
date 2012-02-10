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

#include "KCupsRequestJobs.h"

#include "KCupsConnection.h"

#include <cups/cups.h>

#include <KLocale>

#include <KDebug>

KCupsRequestJobs::KCupsRequestJobs()
{
}

void KCupsRequestJobs::cancelJob(const QString &destName, int jobId)
{
    if (KCupsConnection::readyToStart()) {
        do {
            cupsCancelJob(destName.toUtf8(), jobId);
            setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        } while (KCupsConnection::retryIfForbidden());
        setFinished();
    } else {
        invokeMethod("cancelJob", destName, jobId);
    }
}

void KCupsRequestJobs::holdJob(const QString &destName, int jobId)
{
    if (KCupsConnection::readyToStart()) {
        QHash<QString, QVariant> request;
        request["printer-name"] = destName;
        request["job-id"] = jobId;
        m_retArguments = KCupsConnection::request(IPP_HOLD_JOB,
                                                  "/jobs/",
                                                  request,
                                                  false);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("holdJob", destName, jobId);
    }
}

void KCupsRequestJobs::releaseJob(const QString &destName, int jobId)
{
    if (KCupsConnection::readyToStart()) {
        QHash<QString, QVariant> request;
        request["printer-name"] = destName;
        request["job-id"] = jobId;
        m_retArguments = KCupsConnection::request(IPP_RELEASE_JOB,
                                                  "/jobs/",
                                                  request,
                                                  false);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("releaseJob", destName, jobId);
    }
}

void KCupsRequestJobs::moveJob(const QString &fromDestname, int jobId, const QString &toDestname)
{
    if (jobId < -1 || fromDestname.isEmpty() || toDestname.isEmpty() || jobId == 0) {
        qWarning() << "Internal error, invalid input data" << jobId << fromDestname << toDestname;
        setFinished();
        return;
    }

    if (KCupsConnection::readyToStart()) {
        QHash<QString, QVariant> request;
        request["printer-name"] = fromDestname;
        request["job-id"] = jobId;
        request["job-printer-uri"] = toDestname;

        m_retArguments = KCupsConnection::request(CUPS_MOVE_JOB,
                                                  "/jobs/",
                                                  request,
                                                  false);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("moveJob", fromDestname, jobId, toDestname);
    }
}
