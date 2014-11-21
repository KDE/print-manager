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

#include "PrintQueue.h"

#include <config.h>

#include <QCommandLineParser>
#include <QCommandLineOption>

#include <KLocalizedString>
#include <KAboutData>
#include <KDBusService>

int main(int argc, char **argv)
{
    PrintQueue app(argc, argv);
    app.setOrganizationDomain("org.kde");

    KAboutData about("PrintQueue",
                     i18n("Print Queue"),
                     PM_VERSION,
                     i18n("Print Queue"),
                     KAboutLicense::GPL,
                     i18n("(C) 2010-2013 Daniel Nicoletti"));

    about.addAuthor(QStringLiteral("Daniel Nicoletti"), QString(), "dantti12@gmail.com");

    KAboutData::setApplicationData(about);
    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addPositionalArgument("queue", i18n("Show printer queue(s)"));
    parser.process(app);
    about.processCommandLine(&parser);

    QObject::connect(&service, &KDBusService::activateRequested, &app, &PrintQueue::showQueues);

    app.showQueues(parser.positionalArguments());

    return app.exec();
}
