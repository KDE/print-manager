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

#include "AddPrinter.h"

#include "AddPrinterInterface.h"

#include <KCmdLineArgs>
#include <KDebug>

AddPrinter::AddPrinter() :
    KUniqueApplication()
{
    setQuitOnLastWindowClosed(true);
    m_pqInterface = new AddPrinterInterface(this);
    connect(m_pqInterface, SIGNAL(quit()), this, SLOT(quit()));
}

int AddPrinter::newInstance()
{
    // Example of data
    // "direct"
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    // "Samsung SCX-4200 Series"
    // "Samsung SCX-4200 Series"
    // "usb://Samsung/SCX-4200%20Series"
    // ""
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("add")) {
        QString printer;
        QString deviceId;

//        printer = "Samsung SCX-3400 Series";
//        deviceId = "MFG:Samsung;CMD:SPL,FWV,PIC,BDN,EXT;MDL:SCX-3400 Series;CLS:PRINTER;MODE:SCN,SPL3,R000105;STATUS:BUSY;";

        printer = "Samsung SCX-4200 Series";
        deviceId = "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;";

//        printer = "HP PSC 1400 series";
//        deviceId = "MFG:HP;MDL:PSC 1400 series;DES:;CMD:LDL,MLC,PML,DYN;";

        m_pqInterface->AddPrinter(0);

//        m_pqInterface->NewPrinterFromDevice(0, printer, deviceId);

//        m_pqInterface->ChangePPD(0, "foo");
//        m_pqInterface->ChangePPD(0, "Samsung_SCX-3400_Series");
    }
    args->clear();
    return 0;
}

AddPrinter::~AddPrinter()
{
}
