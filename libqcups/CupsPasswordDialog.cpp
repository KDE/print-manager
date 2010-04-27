/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#include "CupsPasswordDialog.h"

#include <KLocale>
#include <KDebug>

CupsPasswordDialog::CupsPasswordDialog(QEventLoop *loop,
                                       const QString &username,
                                       bool showErrorMsg,
                                       QWidget *parent)
 : KPasswordDialog(parent, KPasswordDialog::ShowUsernameLine), m_loop(loop)
{
    setModal(true);
    setPrompt(i18n("Enter an username and a password to complete the task"));
    setUsername(username);
    if (showErrorMsg) {
        showErrorMessage(QString(), KPasswordDialog::UsernameError);
        showErrorMessage(i18n("Wrong username or password"), KPasswordDialog::PasswordError);
    }
}

void CupsPasswordDialog::slotButtonClicked(int button)
{
    kDebug() <<  button << KDialog::Ok;
    if (button == KDialog::Ok) {
        m_loop->setProperty("username", username());
        m_loop->setProperty("password", password());
        m_loop->setProperty("canceled", false);
    } else {
        // the dialog was canceled
        m_loop->setProperty("canceled", true);
        kDebug()<< "Password Dialog Canceled Finish2";
    }
    m_loop->exit();
    kDebug() << "END THREAD LOOP";
    KDialog::slotButtonClicked(button);
}
