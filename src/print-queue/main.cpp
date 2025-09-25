/*
 *   SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QQuickWindow>
#include <QWindow>

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>
#include <KWindowSystem>
#include <KirigamiAppDefaults>

#include "PrintQueue.h"
#include "config.h"

using namespace Qt::StringLiterals;

static void raiseWindow(QWindow *window)
{
    KWindowSystem::updateStartupId(window);
    KWindowSystem::activateWindow(window);
}

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    QApplication app(argc, argv);
    KirigamiAppDefaults::apply(&app);

    app.setWindowIcon(QIcon::fromTheme(u"printer"_s));
    KLocalizedString::setApplicationDomain("org.kde.plasma.printqueue");

    KAboutData aboutData(QStringLiteral("printqueue"),
                         i18nc("@info:credit", "Plasma Print Queue Manager"),
                         QStringLiteral(PM_VERSION),
                         i18nc("@info:credit", "Manage printer job queues"),
                         KAboutLicense::GPL_V2,
                         i18n("© KDE Community"),
                         QString(),
                         QString(),
                         QStringLiteral("https://bugs.kde.org/enter_bug.cgi?format=guided&product=systemsettings&component=kcm_printer_manager"));

    aboutData.addAuthor(i18nc("@info:credit", "Mike Noe"), i18nc("@info:credit", "Developer"), u"noeerover@gmail.com"_s);
    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    aboutData.setDesktopFileName(u"org.kde.plasma.printqueue"_s);
    KAboutData::setApplicationData(aboutData);

    KDBusService service(KDBusService::Unique);

    QString dest;
    {
        QCommandLineParser parser;
        aboutData.setupCommandLine(&parser);
        parser.addPositionalArgument(QLatin1String("queue"), i18n("Show printer queue"));
        parser.process(app);
        aboutData.processCommandLine(&parser);

        if (parser.positionalArguments().count() == 1) {
            dest = parser.positionalArguments().at(0);
        }
    }

    PrintQueue pq(dest);
    QObject::connect(&service, &KDBusService::activateRequested, &app, [&pq](const QStringList &arguments) {
        if (auto win = pq.mainWindow()) {
            raiseWindow(win);
        }
        if (arguments.count() > 1) {
            pq.init(arguments.at(1)); // executable is first param
        } else {
            pq.init({});
        }
    });

    return app.exec();
}
