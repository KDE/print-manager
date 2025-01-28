// SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QCommandLineParser>

#include <DevicesModel.h>
#include <JobModel.h>
#include <PPDModel.h>
#include <PrinterModel.h>

using namespace Qt::Literals::StringLiterals;

class ModelTests : public QObject
{
    Q_OBJECT

public:
    ModelTests(QObject *parent = nullptr)
        : QObject(parent)
    {
    };

public Q_SLOTS:

    void printerModel()
    {
        // FIXME: model loads on creation, will change for Plasma 6.4
        PrinterModel printers;
        QObject::connect(&printers, &PrinterModel::error, this, [this,&printers](int err, const QString &m, const QString &m1) {
            if (err != 0) { // err == 0 on success
                qDebug() << "Failed:" << m << m1;
            } else {
                qDebug() << "Printer Model loaded, rows:" << printers.rowCount();
            }
            Q_EMIT done();
        });
        Q_EMIT running();
    }

    void jobModel()
    {
        // FIXME: We need a signal from the job model that it's loaded
        JobModel jobs;
        jobs.init("test"_L1);
        Q_EMIT done();
    }

    void ppdModel()
    {
        PPDModel ppds;
        QObject::connect(&ppds, &PPDModel::error, this, [this](const QString &m) {
            qDebug() << "Error from PPDModel:" << m;
            Q_EMIT done();
        });
        QObject::connect(&ppds, &PPDModel::loaded, this, [this,&ppds]() {
            if (ppds.rowCount() == 0) {
                qDebug() << "Failed: No rows in PPD model";
            } else {
                qDebug() << "PPD row count:" << ppds.rowCount();
            }

            Q_EMIT done();
        });

        ppds.load();
        Q_EMIT running();
    }

    void devicesModel()
    {
        DevicesModel devices;
        QObject::connect(&devices, &DevicesModel::loaded, this, [this,&devices]() {
            if (devices.rowCount() > 0)
                qDebug() << "Failed: No rows in Devices destinations model";
            Q_EMIT done();
        });

        devices.update();
        Q_EMIT running();
    }

Q_SIGNALS:
    void done();
    void running();
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Test libkcups models"_s);

    QCommandLineOption printerModel(u"p"_s, u"Load PrinterModel"_s);
    parser.addOption(printerModel);
    QCommandLineOption jobModel(u"j"_s, u"Load JobModel"_s);
    parser.addOption(jobModel);
    QCommandLineOption ppdModel(u"s"_s, u"Load PPD Model"_s);
    parser.addOption(ppdModel);
    QCommandLineOption devicesModel(u"d"_s, u"Load Devices Model"_s);
    parser.addOption(devicesModel);

    parser.process(app);

    const auto models = new ModelTests(qApp);
    QObject::connect(models, &ModelTests::done, qApp, [] {
        qDebug() << "TEST DONE";
        QCoreApplication::quit(); }
    , Qt::QueuedConnection);

    QObject::connect(models, &ModelTests::running, qApp, [models] {
        qDebug() << "TEST RUNNING";
        QEventLoop loop;
        QObject::connect(models, &ModelTests::done, qApp, [&loop]{
            qDebug() << "WAIT EVENT LOOP Quitting";
            loop.quit();
        });
        loop.exec();
    });

    if (parser.isSet(printerModel)) {
        models->printerModel();
    } else if (parser.isSet(jobModel)) {
        models->jobModel();
    } else if (parser.isSet(ppdModel)) {
        models->ppdModel();
    } else if (parser.isSet(devicesModel)) {
        models->devicesModel();
    } else {
        parser.showHelp();
    }

    return app.exec();
}

#include "modeltests.moc"
