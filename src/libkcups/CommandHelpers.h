/*
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <kcups_export.h>
#include <qqmlregistration.h>

class KCupsRequest;
/**
 * @brief In addition to the Qml singleton, this class provides
 * an instance for any c++ components that need printer commands.
 * Referencing the Qml Singleton and C++ global in the same process
 * will create two distinct instances.
 *
 * setupRequest() helpers provide a callback when the request is
 * done and/or a callback for error.  Also handles the request
 * delete.
 *
 * Each command has a "done" signal emitted if there is no error
 */
class KCUPS_EXPORT PrinterCommands : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /**
     * @brief
     * @return instance
     */
    static PrinterCommands *instance();

    explicit PrinterCommands(QObject *parent = nullptr);
    ~PrinterCommands() override;

#ifdef LIBCUPS_VERSION_2
    enum PPDType {
        Manual = 0,
        Auto,
        Custom
    };
    Q_ENUM(PPDType)
#endif
    Q_INVOKABLE void pausePrinter(const QString &printerName);
    Q_INVOKABLE void resumePrinter(const QString &printerName);
    Q_INVOKABLE void removePrinter(const QString &printerName);

    Q_INVOKABLE void setDefault(const QString &printerName);
    Q_INVOKABLE void setShared(const QString &printerName, bool isClass, bool shared = true);
    Q_INVOKABLE void setAcceptingJobs(const QString &printerName, bool accept = true);

    Q_INVOKABLE void printTestPage(const QString &printerName, bool isClass);
    Q_INVOKABLE void printSelfTestPage(const QString &printerName);
    Q_INVOKABLE void cleanPrintHeads(const QString &printerName);

    Q_INVOKABLE void savePrinter(const QString &printerName, const QVariantMap &saveArgs, bool isClass);

Q_SIGNALS:
    void error(int lastError, const QString &errorTitle, const QString &errorMsg);
    void removeDone();
    void saveDone(bool forceRefresh);
    void pauseDone();
    void resumeDone();
    void defaultDone();
    void sharedDone();
    void testDone();
    void cleanDone();
    void acceptDone();

private:
    using SignalFunc = void (PrinterCommands::*)();
    using RequestFunc = void (PrinterCommands::*)(KCupsRequest *);
    using StdRequestFunc = std::function<void(KCupsRequest *)>;

    void defaultErrorCB(KCupsRequest *request);
    KCupsRequest *setupRequest(SignalFunc finished_cb, RequestFunc error_cb = nullptr);
    KCupsRequest *setupRequest(RequestFunc finished_cb, RequestFunc error_cb = nullptr);
    KCupsRequest *setupRequest(StdRequestFunc finished_cb, StdRequestFunc error_cb = nullptr);
};
