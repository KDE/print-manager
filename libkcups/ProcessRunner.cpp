/***************************************************************************
 *   Copyright (C) 2015 Jan Grulich                                        *
 *   <jgrulich@redhat.com>                                                 *
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

#include "ProcessRunner.h"

#include <QProcess>

ProcessRunner::ProcessRunner(QObject* parent)
{
    Q_UNUSED(parent);
}

void ProcessRunner::configurePrinter(const QString& printerName)
{
    QProcess::startDetached(QLatin1String("configure-printer"), {printerName});
}

void ProcessRunner::openPrintQueue(const QString& printerName)
{
    QProcess::startDetached(QLatin1String("kde-print-queue"), {printerName});
}

void ProcessRunner::openPrintKCM()
{
    QProcess::startDetached(QLatin1String("kcmshell5"), {QLatin1String("kcm_printer_manager")});
}

#include "moc_ProcessRunner.cpp"
