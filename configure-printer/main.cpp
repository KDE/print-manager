/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ConfigurePrinter.h"

#include <config.h>

#include <KLocalizedString>
#include <KAboutData>
#include <KDBusService>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include "Debug.h"

int main(int argc, char **argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    ConfigurePrinter app(argc, argv);
    app.setOrganizationDomain(QLatin1String("org.kde"));

    KAboutData aboutData(QLatin1String("ConfigurePrinter"),
                     i18n("Configure Printer"),
                     QLatin1String(PM_VERSION),
                     i18n("ConfigurePrinter"),
                     KAboutLicense::GPL,
                     i18n("(C) 2010-2018 Daniel Nicoletti"));
    aboutData.addAuthor(QStringLiteral("Daniel Nicoletti"), QString(), QLatin1String("dantti12@gmail.com"));
    aboutData.addAuthor(QStringLiteral("Jan Grulich"), i18n("Port to Qt 5 / Plasma 5"), QStringLiteral("jgrulich@redhat.com"));

    KAboutData::setApplicationData(aboutData);
    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(QLatin1String("printer"), i18n("Printer to be configured"));
    parser.process(app);
    aboutData.processCommandLine(&parser);

    const QStringList args = parser.positionalArguments();
    if (args.count() == 1) {
        QString printerName = args.at(0);
        app.configurePrinter(printerName);
    } else {
        qCWarning(PM_CONFIGURE_PRINTER) << "No printer was specified";
        parser.showHelp(1);
    }

    return app.exec();
}
