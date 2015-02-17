/***************************************************************************
 *   Copyright (C) 2012-2013 by Daniel Nicoletti <dantti12@gmail.com>      *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "qmlplugins.h"

#include <QQuickItem>

#include <PrinterModel.h>
#include <PrinterSortFilterModel.h>
#include <JobModel.h>
#include <JobSortFilterModel.h>
#include <ProcessRunner.h>

void QmlPlugins::registerTypes(const char* uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.printmanager"));
    qmlRegisterType<PrinterModel>(uri, 0, 2, "PrinterModel");
    qmlRegisterType<PrinterSortFilterModel>(uri, 0, 2, "PrinterSortFilterModel");
    qmlRegisterType<JobModel>(uri, 0, 2, "JobModel");
    qmlRegisterType<JobSortFilterModel>(uri, 0, 2, "JobSortFilterModel");
    qmlRegisterType<ProcessRunner>(uri, 0, 2, "ProcessRunner");
}
