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

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

#include <KLocalizedString>
#include <KAboutData>

int main(int argc, char **argv)
{
    AddPrinter app(argc, argv);
    app.setOrganizationDomain("org.kde");

    KAboutData about("kde-add-printer",
                     i18n("Add Printer"),
                     PM_VERSION,
                     i18n("Tool for adding new printers"),
                     KAboutLicense::GPL,
                     i18n("(C) 2010-2018 Daniel Nicoletti"));

    about.addAuthor(i18n("Daniel Nicoletti"), QString(), "dantti12@gmail.com");
    about.addAuthor(QStringLiteral("Lukáš Tinkl"), i18n("Port to Qt 5 / Plasma 5"), QStringLiteral("ltinkl@redhat.com"));
    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption({"w", "parent-window"}, i18n("Parent Window ID"), "wid"));
    parser.addOption(QCommandLineOption("add-printer", i18n("Add a new printer")));
    parser.addOption(QCommandLineOption("add-class", i18n("Add a new printer class")));
    parser.addOption(QCommandLineOption("change-ppd", i18n("Changes the PPD of a given printer"), "printer-name"));
    parser.addOption(QCommandLineOption("new-printer-from-device", i18n("Changes the PPD of a given printer/deviceid"),
                                        "printername/deviceid"));

    parser.process(app);
    about.processCommandLine(&parser);

    qulonglong wid = 0;
    if (parser.isSet("w")) {
        wid = parser.value("parent-window").toULongLong();
    }

    if (parser.isSet("add-printer")) {
        app.addPrinter(wid);
    } else if (parser.isSet("add-class")) {
        app.addClass(wid);
    } else if (parser.isSet("change-ppd")) {
        app.changePPD(wid, parser.value("change-ppd"));
    } else if (parser.isSet("new-printer-from-device")) {
        const QString value = parser.value("new-printer-from-device");
        const QStringList values = value.split(QLatin1String("/"));
        if (values.size() == 2) {
            app.newPrinterFromDevice(wid, values.first(), values.last());
        } else {
            qWarning() << "The expected input should be printer/deviceid";
            exit(EXIT_FAILURE);
        }
    } else {
        parser.showHelp(EXIT_FAILURE);
    }

    return app.exec();
}
