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

#include "PrintManagerService.h"

#include "PrintManagerServiceJob.h"

#include <KDebug>

PrintManagerService::PrintManagerService(QObject *parent, const QString &destination) :
    Plasma::Service(parent)
{
    setName("printmanager");
    setDestination(destination);
    if (destination.isEmpty()) {
        setOperationEnabled("pausePrinter", false);
        setOperationEnabled("resumePrinter", false);
    } else {
        setOperationEnabled("cancelJob", false);
        setOperationEnabled("holdJob", false);
        setOperationEnabled("releaseJob", false);
        setOperationEnabled("moveJob", false);
    }
}

Plasma::ServiceJob* PrintManagerService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    kDebug() << operation << parameters;
    QString dest = destination();
    if (!parameters[QLatin1String("PrinterName")].isNull()) {
        dest = parameters[QLatin1String("PrinterName")].toString();
    }
    return new PrintManagerServiceJob(dest, operation, parameters, this);
}
