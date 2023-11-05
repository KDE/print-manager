/**
 * SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PRINTERMANAGER_H
#define PRINTERMANAGER_H

#include <KQuickConfigModule>
#include <QObject>

#include "KCupsRequest.h"
#include "PrinterSortFilterModel.h"

class PrinterManager : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(PrinterSortFilterModel *printerModel READ printerModel CONSTANT)
    Q_PROPERTY(bool shareConnectedPrinters READ shareConnectedPrinters WRITE setShareConnectedPrinters NOTIFY settingsChanged)
    Q_PROPERTY(bool allowPrintingFromInternet READ allowPrintingFromInternet WRITE setAllowPrintingFromInternet NOTIFY settingsChanged)
    Q_PROPERTY(bool allowRemoteAdmin READ allowRemoteAdmin WRITE setAllowRemoteAdmin NOTIFY settingsChanged)
    Q_PROPERTY(bool allowUserCancelAnyJobs READ allowUserCancelAnyJobs WRITE setAllowUserCancelAnyJobs NOTIFY settingsChanged);

public:
    PrinterManager(QObject *parent, const KPluginMetaData &metaData);
    PrinterSortFilterModel *printerModel() const;

    Q_INVOKABLE void addPrinter();
    Q_INVOKABLE void removePrinter(const QString &name);
    Q_INVOKABLE void configurePrinter(const QString &name);
    Q_INVOKABLE void openPrintQueue(const QString &name);
    Q_INVOKABLE void makePrinterDefault(const QString &name);
    Q_INVOKABLE void makePrinterShared(const QString &name, bool shared, bool isClass);
    Q_INVOKABLE void makePrinterRejectJobs(const QString &name, bool reject);
    Q_INVOKABLE void printTestPage(const QString &name, bool isClass);
    Q_INVOKABLE void printSelfTestPage(const QString &name);
    Q_INVOKABLE void cleanPrintHeads(const QString &name);
    Q_INVOKABLE void pausePrinter(const QString &name);
    Q_INVOKABLE void resumePrinter(const QString &name);

    bool shareConnectedPrinters() const;
    void setShareConnectedPrinters(bool share);

    bool allowPrintingFromInternet() const;
    void setAllowPrintingFromInternet(bool allow);

    bool allowRemoteAdmin() const;
    void setAllowRemoteAdmin(bool allow);

    bool allowUserCancelAnyJobs() const;
    void setAllowUserCancelAnyJobs(bool allow);

Q_SIGNALS:
    void requestError(const QString &errorMessage);
    void removeComplete(bool removed);
    void settingsChanged();

private:
    void requestFinished(KCupsRequest *request);
    void updateServerFinished(KCupsRequest *request);
    void getServerSettings();
    KCupsRequest *setupRequest();
    KCupsRequest *setupServerRequest();

    PrinterSortFilterModel *m_model;
    bool m_shareConnectedPrinters = false;
    bool m_allowPrintingFromInternet = false;
    bool m_allowRemoteAdmin = false;
    bool m_allowUserCancelAnyJob = false;
    bool m_removing = false;
    int m_saveCount = 0;
};

#endif
