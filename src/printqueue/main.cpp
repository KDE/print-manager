/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrintQueue.h"

#include <config.h>

#include <QCommandLineParser>
#include <QCommandLineOption>

#include <KLocalizedString>
#include <KAboutData>
#include <KDBusService>

int main(int argc, char **argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    PrintQueue app(argc, argv);
    app.setOrganizationDomain(QLatin1String("org.kde"));

    KAboutData about(QLatin1String("PrintQueue"),
                     i18n("Print Queue"),
                     QLatin1String(PM_VERSION),
                     i18n("Print Queue"),
                     KAboutLicense::GPL,
                     i18n("(C) 2010-2018 Daniel Nicoletti"));

    about.addAuthor(QStringLiteral("Daniel Nicoletti"), QString(), QLatin1String("dantti12@gmail.com"));
    about.addAuthor(QStringLiteral("Lukáš Tinkl"), i18n("Port to Qt 5 / Plasma 5"), QStringLiteral("ltinkl@redhat.com"));

    KAboutData::setApplicationData(about);
    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    about.setupCommandLine(&parser);
    parser.addPositionalArgument(QLatin1String("queue"), i18n("Show printer queue(s)"));
    parser.process(app);
    about.processCommandLine(&parser);

    QObject::connect(&service, &KDBusService::activateRequested, &app, [&app](const QStringList &arguments) {
        if (!arguments.isEmpty()) {
            app.showQueues(arguments.mid(1)); // strip off executable name
        }
    });

    app.showQueues(parser.positionalArguments());

    return app.exec();
}
