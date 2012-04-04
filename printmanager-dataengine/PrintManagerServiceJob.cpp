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

#include "PrintManagerServiceJob.h"

#include <KDebug>
#include <KCupsRequest.h>

PrintManagerServiceJob::PrintManagerServiceJob(const QString &destination, const QString &operation,
                                               const QMap<QString, QVariant> &parameters, QObject *parent) :
    Plasma::ServiceJob(destination, operation, parameters, parent)
{
    kDebug() << destination << operation << parameters;
}

void PrintManagerServiceJob::start()
{
    kDebug() << operationName() << destination();
    KCupsRequest *request = new KCupsRequest;
    if (operationName() == QLatin1String("cancelJob")) {
        request->cancelJob(destination(),
                           parameters()[QLatin1String("JobId")].toInt());
    } else if (operationName() == QLatin1String("holdJob")) {
        request->holdJob(destination(),
                         parameters()[QLatin1String("JobId")].toInt());
    } else if (operationName() == QLatin1String("releaseJob")) {
        request->releaseJob(destination(),
                            parameters()[QLatin1String("JobId")].toInt());
    } else if (operationName() == QLatin1String("moveJob")) {
        request->moveJob(destination(),
                         parameters()[QLatin1String("JobId")].toInt(),
                         parameters()[QLatin1String("DestinationPrinterName")].toString());
    }
    request->waitTillFinished();
    if (request->hasError()) {
        setError(request->error());
        setErrorText(request->errorMsg());
    }
    request->deleteLater();
    emitResult();
}
