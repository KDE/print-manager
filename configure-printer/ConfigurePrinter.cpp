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
#include "Debug.h"

#include <QTimer>
#include <QCommandLineParser>

ConfigurePrinter::ConfigurePrinter(int & argc, char ** argv) :
    QApplication(argc, argv)
{
    m_cpInterface = new ConfigurePrinterInterface(this);
    connect(m_cpInterface, SIGNAL(quit()), this, SLOT(quit()));
}

int ConfigurePrinter::newInstance()
{
    QCommandLineParser args;
    QCommandLineOption configurePrinterOption("configure-printer");
    args.addOption(configurePrinterOption);
    args.process(this);

    if (args.isSet(configurePrinterOption)) {
        m_cpInterface->ConfigurePrinter(args.value(configurePrinterOption));
    } else {
        // If DBus called the ui list won't be empty
        QTimer::singleShot(500, m_cpInterface, SLOT(RemovePrinter()));
    }

    return 0;
}

ConfigurePrinter::~ConfigurePrinter()
{
}

#include "ConfigurePrinter.moc"
