/***************************************************************************
 *   Copyright (C) 2010 by Glauber M. Dantas                               *
 *   glauber.md@gmail.com                                                  *
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
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

#include "SystemPreferences.h"
#include "ui_SystemPreferences.h"

#include <KCupsRequestServer.h>
#include <KCupsServer.h>

#include <KDebug>

SystemPreferences::SystemPreferences(QWidget *parent) :
    KDialog(parent),
    ui(new Ui::SystemPreferences)
{
    ui->setupUi(mainWidget());
    connect(this, SIGNAL(okClicked()), SLOT(save()));

    KCupsRequestServer *request = new KCupsRequestServer;
    connect(request, SIGNAL(server(KCupsServer)), this, SLOT(server(KCupsServer)));
    request->getServerSettings();
}

SystemPreferences::~SystemPreferences()
{
    delete ui;
}

void SystemPreferences::server(const KCupsServer &server)
{
    KCupsRequestServer *request = qobject_cast<KCupsRequestServer *>(sender());
    request->deleteLater();

    ui->showSharedPrintersCB->setChecked(server.showSharedPrinters());
    ui->shareConnectedPrintersCB->setChecked(server.sharePrinters());
    ui->allowFromInternetCB->setChecked(server.allowPrintingFromInternet());
    ui->allowRemoteAdminCB->setChecked(server.allowRemoteAdmin());
    ui->allowUsrCancelCB->setChecked(server.allowUserCancelAnyJobs());
}

void SystemPreferences::saveFinished()
{
    KCupsRequestServer *request = qobject_cast<KCupsRequestServer *>(sender());
    if (request->hasError()) {
        qWarning() << "Failed to set server settings" << request->errorMsg();
    }
    request->deleteLater();
}

void SystemPreferences::save()
{
    KCupsServer server;
    server.setShowSharedPrinters(ui->showSharedPrintersCB->isChecked());
    server.setSharePrinters(ui->shareConnectedPrintersCB->isChecked());
    server.setAllowPrintingFromInternet(ui->allowFromInternetCB->isChecked());
    server.setAllowRemoteAdmin(ui->allowRemoteAdminCB->isChecked());
    server.setAllowUserCancelAnyJobs(ui->allowUsrCancelCB->isChecked());
    KCupsRequestServer *request = server.commit();
    connect(request, SIGNAL(finished()), this, SLOT(saveFinished()));
}

#include "SystemPreferences.moc"
