/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "ConfigurePrinter.h"

#include "ConfigurePrinterInterface.h"

#include <QTimer>

#include <KCmdLineArgs>
#include <KDebug>

ConfigurePrinter::ConfigurePrinter() :
    KUniqueApplication()
{
    m_cpInterface = new ConfigurePrinterInterface(this);
    connect(m_cpInterface, SIGNAL(quit()), this, SLOT(quit()));
}

int ConfigurePrinter::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString printerName = args->getOption("configure-printer");
    if (!printerName.isEmpty()) {
        m_cpInterface->ConfigurePrinter(printerName);
    } else {
        // If DBus called the ui list won't be empty
        QTimer::singleShot(500, m_cpInterface, SLOT(RemovePrinter()));
    }
    args->clear();

    return 0;
}

ConfigurePrinter::~ConfigurePrinter()
{
}

#include "ConfigurePrinter.moc"
