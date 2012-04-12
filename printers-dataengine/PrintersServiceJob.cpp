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

#include "PrintersServiceJob.h"

#include <KDebug>
#include <KCupsRequest.h>

PrintersServiceJob::PrintersServiceJob(const QString &destination, const QString &operation,
                                               const QMap<QString, QVariant> &parameters, QObject *parent) :
    Plasma::ServiceJob(destination, operation, parameters, parent)
{
}

void PrintersServiceJob::start()
{
    kDebug() << destination() << operationName();
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(finished()), this, SLOT(jobFinished()));
    if (operationName() == QLatin1String("pause")) {
        request->pausePrinter(destination());
    } else if (operationName() == QLatin1String("resume")) {
        request->resumePrinter(destination());
    } else if (operationName() == QLatin1String("rejectJobs")) {
        request->rejectJobs(destination());
    } else if (operationName() == QLatin1String("acceptJobs")) {
        request->acceptJobs(destination());
    } else {
        kWarning() << "Operation not defined!" << operationName();
        request->deleteLater();
        Plasma::ServiceJob::start();
    }
}

void PrintersServiceJob::jobFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    if (request->hasError()) {
        setError(request->error());
        setErrorText(request->errorMsg());
    }
    request->deleteLater();

    emitResult();
}
