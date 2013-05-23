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

#include <config.h>

#include <KDebug>
#include <KLocale>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char **argv)
{
    KAboutData about("add-printer",
                     "print-manager",
                     ki18n("AddPrinter"),
                     PM_VERSION,
                     ki18n("Tool for adding new printers"),
                     KAboutData::License_GPL,
                     ki18n("(C) 2010-2013 Daniel Nicoletti"));

    about.addAuthor(ki18n("Daniel Nicoletti"), KLocalizedString(), "dantti12@gmail.com");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    options.add("w").add("parent-window <wid>", ki18n("Parent Window ID"));
    options.add("add-printer", ki18n("Add a new printer"));
    options.add("add-class", ki18n("Add a new printer class"));
    options.add("change-ppd <printer-name>", ki18n("Changes the PPD of a given printer"));
    options.add("new-printer-from-device <printername/deviceid>", ki18n("Changes the PPD of a given printer/deviceid"));
    KCmdLineArgs::addCmdLineOptions(options);

    qulonglong wid = 0;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("w")) {
        wid = args->getOption("parent-window").toULongLong();
    }

    AddPrinter app;
    if (args->isSet("add-printer")) {
        app.addPrinter(wid);
    } else if (args->isSet("add-class")) {
        app.addClass(wid);
    } else if (args->isSet("change-ppd")) {
        app.changePPD(wid, args->getOption("change-ppd"));
    } else if (args->isSet("new-printer-from-device")) {
        QString value = args->getOption("new-printer-from-device");
        QStringList values = value.split(QLatin1String("/"));
        if (values.size() == 2) {
            app.newPrinterFromDevice(wid, values.first(), values.last());
        } else {
            args->usage("new-printer-from-device");
        }
    } else {
        args->usage();
    }


    return app.exec();
}
