// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QCommandLineParser>

#include <KCupsRequest.h>

using namespace Qt::Literals::StringLiterals;

class CupsServerTest : public QObject
{
    Q_OBJECT

public:
    CupsServerTest(QObject *parent)
        : QObject(parent)
    {
    };

public Q_SLOTS:

    void getServerSettings()
    {
        const auto request = new KCupsRequest();
        request->getServerSettings();
        request->waitTillFinished();

        bool success = false;
        if (request->hasError() && request->error() != IPP_NOT_FOUND) {
            // success = false;
        } else {
            const auto attrs = request->serverSettings().arguments();
            if (!attrs.isEmpty()) {
                success = true;
                for (auto [key, value] : attrs.asKeyValueRange()) {
                    qDebug() << "CUPS admin attribute:" << key << ":" << value;
                }
            }
        }
        request->deleteLater();
        if (!success) {
            qDebug() << "Failed to get CUPS server settings";
        }

        Q_EMIT done();
    }

    void setSharePrinters(bool share) {
        KCupsServer server;
        server.setSharePrinters(share);
        auto request = new KCupsRequest;
        request->setServerSettings(server);
        request->waitTillFinished();

        if (request->error() == IPP_AUTHENTICATION_CANCELED || request->error() == IPP_SERVICE_UNAVAILABLE || request->error() == IPP_INTERNAL_ERROR) {
            qDebug() << QLatin1String("Server Settings Not Saved: (%1): %2").arg(request->serverError(), request->errorMsg());
        } else {
            qDebug() << "Share Printers SAVED!";
        }
        request->deleteLater();
        Q_EMIT done();
    }

Q_SIGNALS:
    void done();
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Test calling for the CUPS server settings"_s);

    QCommandLineOption getSettings(u"s"_s, u"Get server settings"_s);
    parser.addOption(getSettings);

    QCommandLineOption setSharePrinters(QStringList() << u"r"_s,
                u"Set the share printers server flag (0 or 1)"_s, u"flag"_s);
    parser.addOption(setSharePrinters);

    parser.process(app);

    const auto cups = new CupsServerTest(qApp);
    QObject::connect(cups, &CupsServerTest::done, &app, &QCoreApplication::quit, Qt::QueuedConnection);

    if (parser.isSet(getSettings)) {
        cups->getServerSettings();
    } else if (parser.isSet(setSharePrinters)) {
        cups->setSharePrinters(parser.value(setSharePrinters).toInt());
    } else {
        parser.showHelp();
    }

    return app.exec();
}

#include "cupsservertests.moc"
