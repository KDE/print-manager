/*
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <kcups_export.h>
#include <qqmlregistration.h>

#include <KCupsRequest.h>

/**
 * @brief Provide printer commands with helpers
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
    explicit PrinterCommands(QObject *parent = nullptr);
    ~PrinterCommands() override;

#ifdef LIBCUPS_VERSION_2
    enum class PPDType {
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
    using StdRequestCB = std::function<void(KCupsRequest *)>;
    KCupsRequest *setupRequest(StdRequestCB success_cb, StdRequestCB error_cb = nullptr);
};
