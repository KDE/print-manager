/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti12@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef PRINT_KCM_H
#define PRINT_KCM_H

#include <KCModule>
#include <KIcon>

#include <QAction>

#include <KCupsServer.h>

namespace Ui {
    class PrintKCM;
}
class KCupsRequest;
class PrinterModel;
class PrintKCM : public KCModule
{
    Q_OBJECT
public:
    PrintKCM(QWidget *parent, const QVariantList &args);
    ~PrintKCM();

private slots:
    void update();
    void on_addTB_clicked();
    void addClass();
    void on_removeTB_clicked();

    void error(int lastError, const QString &errorTitle, const QString &errorMsg);
    void showInfo(const KIcon &icon, const QString &title, const QString &comment, bool showAddPrinter, bool showToolButtons);

    void getServerSettings();
    void getServerSettingsFinished();
    void updateServerFinished();
    void systemPreferencesTriggered();

private:
    Ui::PrintKCM *ui;
    PrinterModel *m_model;
    int m_lastError;

    KCupsRequest *m_serverRequest;
    QAction *m_showSharedPrinters;
    QAction *m_shareConnectedPrinters;
    QAction *m_allowPrintringFromInternet;
    QAction *m_allowRemoteAdmin;
    QAction *m_allowUsersCancelAnyJob;
};

#endif
