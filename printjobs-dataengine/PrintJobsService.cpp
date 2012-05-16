/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti                                *
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

#include "PrintJobsService.h"

#include "PrintJobsServiceJob.h"

#include <KDebug>

PrintJobsService::PrintJobsService(QObject *parent, const QString &destination) :
    Plasma::Service(parent)
{
    setName("org.kde.printjobs");
    setDestination(destination);
}

Plasma::ServiceJob* PrintJobsService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    // JobId was stored on the destination
    parameters[QLatin1String("JobId")] = destination().toInt();

    // The printer name that holds the jobs was passed as a parameter
    QString printer = parameters[QLatin1String("PrinterName")].toString();

    return new PrintJobsServiceJob(printer, operation, parameters, this);
}
