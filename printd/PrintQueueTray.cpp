/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "PrintQueueTray.h"

#include <KMenu>
#include <KLocale>
#include <KActionCollection>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

PrintQueueTray::PrintQueueTray(QObject *parent)
 : KStatusNotifierItem(parent)
{
    setCategory(Hardware);
    setIconByName("printer");
    setStatus(Active);

    // Remove standard quit action, as it would quit all of KDED
    KActionCollection *actions = actionCollection();
    actions->removeAction(actions->action(KStandardAction::name(KStandardAction::Quit)));
    connect(contextMenu(), SIGNAL(triggered(QAction *)),
            this, SLOT(openQueue(QAction *)));
    connect(this, SIGNAL(activateRequested(bool, const QPoint &)), this, SLOT(openDefaultQueue()));
}

PrintQueueTray::~PrintQueueTray()
{
}

void PrintQueueTray::connectToLauncher(const QString &destName)
{
    m_destName = destName;
}

void PrintQueueTray::openDefaultQueue()
{
    QAction action(this);
    action.setData(m_destName);
    openQueue(&action);
}

void PrintQueueTray::openQueue(QAction *action)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.PrintQueue",
                                             "/",
                                             "org.kde.PrintQueue",
                                             QLatin1String("ShowQueue"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(action->data().toString());
    QDBusConnection::sessionBus().call(message);
}
