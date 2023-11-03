/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINT_KCM_H
#define PRINT_KCM_H

#include <KCModule>
#include <KPluginFactory>

#include <QIcon>
#include <QAction>

#include <KCupsServer.h>

namespace Ui {
class PrintKCM;
}
class KCupsRequest;
class PrinterModel;
class PrinterSortFilterModel;
class PrintKCM : public KCModule
{
    Q_OBJECT
public:
    explicit PrintKCM(QObject *parent);
    ~PrintKCM() override;

private slots:
    void update();
    void on_addTB_clicked();
    void addClass();
    void on_removeTB_clicked();

    void error(int lastError, const QString &errorTitle, const QString &errorMsg);
    void showInfo(const QIcon &icon, const QString &title, const QString &comment, bool showAddPrinter, bool showToolButtons);

    void getServerSettings();
    void getServerSettingsFinished(KCupsRequest *request);
    void updateServerFinished(KCupsRequest *request);
    void systemPreferencesTriggered();

private:
    Ui::PrintKCM *ui;
    PrinterModel *m_model;
    PrinterSortFilterModel *m_sortModel;
    int m_lastError = -1; // Force the error to run on the first time

    KCupsRequest *m_serverRequest = nullptr;
    QAction *m_showSharedPrinters;
    QAction *m_shareConnectedPrinters;
    QAction *m_allowPrintringFromInternet;
    QAction *m_allowRemoteAdmin;
    QAction *m_allowUsersCancelAnyJob;
};

#endif
