/*
 * SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QObject>
#include <QPointer>
#include <QProcess>

#include <KCupsRequest.h>

using namespace Qt::Literals::StringLiterals;

constexpr QLatin1String TEST_PRINTER = "test"_L1;

class TestHelpers : public QObject
{
    Q_OBJECT

public:
    explicit TestHelpers(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~TestHelpers()
    {
        if (m_process) {
            m_process->close();
            m_process->deleteLater();
            qDebug() << "Stopping IPPServer";
        }
    }

    bool startIPPServer()
    {
        m_process = new QProcess();
        QObject::connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
            qDebug() << m_process->readAllStandardOutput();
        });
        QObject::connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
            qDebug() << m_process->readAllStandardError();
        });

        m_process->start(u"ippserver"_s, {u"-vvv"_s, u"-C"_s, u"../bin/scripts/mock"_s, u"-r"_s, u"_print"_s});
        bool started = m_process->waitForStarted();
        if (!started) {
            qDebug() << "IPPServer not found";
            m_process->deleteLater();
            m_process.clear();
            return false;
        }
        return true;
    }

    KCupsRequest *getPrinterListRequest()
    {
        const auto req = new KCupsRequest();
        req->getPrinters({KCUPS_PRINTER_NAME,
                          KCUPS_PRINTER_STATE,
                          KCUPS_PRINTER_STATE_MESSAGE,
                          KCUPS_PRINTER_IS_SHARED,
                          KCUPS_PRINTER_IS_ACCEPTING_JOBS,
                          KCUPS_PRINTER_TYPE,
                          KCUPS_PRINTER_LOCATION,
                          KCUPS_PRINTER_INFO,
                          KCUPS_PRINTER_MAKE_AND_MODEL,
                          KCUPS_PRINTER_COMMANDS,
                          KCUPS_DEVICE_URI,
                          KCUPS_PRINTER_URI_SUPPORTED,
                          KCUPS_MEMBER_NAMES});
        return req;
    }

    KCupsRequest *getPrinterRequest(const QString &name = {})
    {
        const auto req = new KCupsRequest();
        req->getPrinterAttributes(name.isEmpty() ? TEST_PRINTER : name,
                                  false,
                                  {KCUPS_PRINTER_NAME,
                                   KCUPS_PRINTER_STATE,
                                   KCUPS_PRINTER_STATE_MESSAGE,
                                   KCUPS_PRINTER_IS_SHARED,
                                   KCUPS_PRINTER_IS_ACCEPTING_JOBS,
                                   KCUPS_PRINTER_TYPE,
                                   KCUPS_PRINTER_LOCATION,
                                   KCUPS_PRINTER_INFO,
                                   KCUPS_PRINTER_MAKE_AND_MODEL,
                                   KCUPS_PRINTER_COMMANDS,
                                   KCUPS_DEVICE_URI,
                                   KCUPS_PRINTER_URI_SUPPORTED,
                                   KCUPS_MEMBER_NAMES});
        return req;
    }

private:
    QPointer<QProcess> m_process;
};
