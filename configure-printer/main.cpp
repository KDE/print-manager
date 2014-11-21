/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
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

#include <config.h>

#include <KDebug>
#include <KLocale>
#include <KAboutData>
#include <KDBusService>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char **argv)
{
    ConfigurePrinter app(argc, argv);
    app.setOrganizationDomain("org.kde");

    KAboutData aboutData("ConfigurePrinter",
                     i18n("Configure Printer"),
                     PM_VERSION,
                     i18n("ConfigurePrinter"),
                     KAboutLicense::GPL,
                     i18n("(C) 2010-2013 Daniel Nicoletti"));
    aboutData.addAuthor("Daniel Nicoletti", QString(), "dantti12@gmail.com");

    KAboutData::setApplicationData(aboutData);
    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addPositionalArgument("printer", i18n("Printer to be configured"));
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QStringList args = parser.positionalArguments();
    if (args.count() == 1) {
        QString printerName = args.at(0);
        app.configurePrinter(printerName);
    } else {
        qDebug() << "No printer was specified";
        return 1;
    }

    return app.exec();
}
