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
    app.setOrganizationDomain(QLatin1String("org.kde"));

    KAboutData about(QLatin1String("kde-add-printer"),
                     i18n("Add Printer"),
                     QLatin1String(PM_VERSION),
                     i18n("Tool for adding new printers"),
                     KAboutLicense::GPL,
                     i18n("(C) 2010-2018 Daniel Nicoletti"));

    about.addAuthor(QLatin1String("Daniel Nicoletti"), QString(), QLatin1String("dantti12@gmail.com"));
    about.addAuthor(QStringLiteral("Lukáš Tinkl"), i18n("Port to Qt 5 / Plasma 5"), QStringLiteral("ltinkl@redhat.com"));
    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.addVersionOption();
    parser.addHelpOption();

    QCommandLineOption parentWindowOpt({QLatin1String("w"), QLatin1String("parent-window")}, i18n("Parent Window ID"), QLatin1String("wid"));
    parser.addOption(parentWindowOpt);

    QCommandLineOption addPrinterOpt(QLatin1String("add-printer"), i18n("Add a new printer"));
    parser.addOption(addPrinterOpt);

    QCommandLineOption addClassOpt(QLatin1String("add-class"), i18n("Add a new printer class"));
    parser.addOption(addClassOpt);

    QCommandLineOption changePpdOpt(QLatin1String("change-ppd"), i18n("Changes the PPD of a given printer"), QLatin1String("printer-name"));
    parser.addOption(changePpdOpt);

    QCommandLineOption newPrinterDevOpt(QLatin1String("new-printer-from-device"), i18n("Changes the PPD of a given printer/deviceid"),
                                        QLatin1String("printername/deviceid"));
    parser.addOption(newPrinterDevOpt);

    parser.process(app);
    about.processCommandLine(&parser);

    qulonglong wid = 0;
    if (parser.isSet(parentWindowOpt)) {
        wid = parser.value(parentWindowOpt).toULongLong();
    }

    if (parser.isSet(addPrinterOpt)) {
        app.addPrinter(wid);
    } else if (parser.isSet(addClassOpt)) {
        app.addClass(wid);
    } else if (parser.isSet(changePpdOpt)) {
        app.changePPD(wid, parser.value(changePpdOpt));
    } else if (parser.isSet(newPrinterDevOpt)) {
        const QString value = parser.value(newPrinterDevOpt);
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
