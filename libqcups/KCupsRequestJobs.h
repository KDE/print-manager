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

#ifndef KCUPSREQUESTJOBS_H
#define KCUPSREQUESTJOBS_H

#include <QObject>
#include <QEventLoop>
#include <QVariant>

#include <kdemacros.h>

#include "KCupsRequestInterface.h"

class KDE_EXPORT KCupsRequestJobs : public KCupsRequestInterface
{
    Q_OBJECT
public:
    explicit KCupsRequestJobs();

public slots:
    /**
     * Cancels tries to cancel a given job
     * @param destName the destination name (printer)
     * @param jobId the job identification
     */
    void cancelJob(const QString &destName, int jobId);

    /**
     * Holds the printing of a given job
     * @param destName the destination name (printer)
     * @param jobId the job identification
     */
    void holdJob(const QString &destName, int jobId);

    /**
     * Holds the printing of a given job
     * @param destName the destination name (printer)
     * @param jobId the job identification
     */
    void releaseJob(const QString &destName, int jobId);

    /**
     * Holds the printing of a given job
     * @param fromDestName the destination name which holds the job
     * @param jobId the job identification
     * @param toDestName the destination to hold the job
     */
    void moveJob(const QString &fromDestname, int jobId, const QString &toDestName);
};

#endif // KCUPSREQUESTJOBS_H
