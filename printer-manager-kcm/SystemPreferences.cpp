/***************************************************************************
 *   Copyright (C) 2010 by Glauber M. Dantas                               *
 *   glauber.md@gmail.com                                                  *
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

#include <QCups.h>

#include <KDebug>

SystemPreferences::SystemPreferences(QWidget *parent)
 : KDialog(parent)
{
    setupUi(mainWidget());
    connect(this,SIGNAL(okClicked()),SLOT(save()));
    QCups::Result *result = QCups::adminGetServerSettings();
    result->waitTillFinished();
    QHash<QString, QString> values = result->hashStrStr();
    result->deleteLater();
    if(values["_remote_printers"] == "1") {
      showSharedPrintersCB->setChecked(true);
    }
    m_values["_remote_printers"] = values["_remote_printers"];
    if (values["_share_printers"] == "1") {
      shareConnectedPrintersCB->setChecked(true);
    }
    m_values["_share_printers"] = values["_share_printers"];
    if (values["_remote_any"] == "1") {
      allowFromInternetCB->setChecked(true);
    }
    m_values["_remote_any"] = values["_remote_any"];
    if (values["_remote_admin"] == "1") {
      allowRemoteAdminCB->setChecked(true);
    }
    m_values["_remote_admin"] = values["_remote_admin"];
    // TODO Need to read Cups FAQ regarding Kerberos Auth
    // (Negotiate, Basic, etc)
    if (values["DefaultAuthType"] != "Basic") {
      useKRBAuthCB->setChecked(true);
    }
    // m_values["DefaultAuthType"] = values["DefaultAuthType"];
    if (values["_user_cancel_any"] == "1") {
      allowUsrCancelCB->setChecked(true);
    }
    m_values["_user_cancel_any"] = values["_user_cancel_any"];
}

void SystemPreferences::save() {
   QHash<QString, QString> userValues;
   if(showSharedPrintersCB->isChecked()) {
       userValues["_remote_printers"] = "1";
   } else {
       userValues["_remote_printers"] = "0";
   }
   if(shareConnectedPrintersCB->isChecked()) {
       userValues["_share_printers"] = "1";
   } else {
       userValues["_share_printers"] = "0";
   }
   if(allowFromInternetCB->isChecked()) {
       userValues["_remote_any"] = "1";
   } else {
       userValues["_remote_any"] = "0";
   }
   if(allowRemoteAdminCB->isChecked()) {
       userValues["_remote_admin"] = "1";
   } else {
       userValues["_remote_admin"] = "0";
   }
//   if(useKRBAuthCB->isChecked()) {
//       userValues["DefaultAuthType"] = "1";
//   } else {
//       userValues["DefaultAuthType"] = "0";
//   }
   if(allowUsrCancelCB->isChecked()) {
       userValues["_user_cancel_any"] = "1";
   } else {
       userValues["_user_cancel_any"] = "0";
   }

//    kDebug() << userValues;
//    kDebug() << m_values;
//    kDebug() << (userValues == m_values);

   if(userValues != m_values) {
       QCups::Result *ret = QCups::adminSetServerSettings(userValues);
       ret->waitTillFinished();
       ret->deleteLater();
   }
}

SystemPreferences::~SystemPreferences()
{
  // Destrutor
}

#include "SystemPreferences.moc"
