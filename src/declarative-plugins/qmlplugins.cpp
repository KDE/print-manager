/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmlplugins.h"

#include <QQuickItem>

#include <PrinterModel.h>
#include <PrinterSortFilterModel.h>
#include <JobModel.h>
#include <JobSortFilterModel.h>
#include <ProcessRunner.h>

void QmlPlugins::registerTypes(const char* uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.printmanager"));
    qmlRegisterType<PrinterModel>(uri, 0, 2, "PrinterModel");
    qmlRegisterType<PrinterSortFilterModel>(uri, 0, 2, "PrinterSortFilterModel");
    qmlRegisterType<JobModel>(uri, 0, 2, "JobModel");
    qmlRegisterType<JobSortFilterModel>(uri, 0, 2, "JobSortFilterModel");
    qmlRegisterType<ProcessRunner>(uri, 0, 2, "ProcessRunner");
}

#include "moc_qmlplugins.cpp"
