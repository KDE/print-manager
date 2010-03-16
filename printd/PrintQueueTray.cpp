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

#include <KIcon>
#include <KMenu>
#include <KLocale>

#include <QtCore/QSignalMapper>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <KDebug>


PrintQueueTray::PrintQueueTray(QObject *parent)
 : KStatusNotifierItem(parent)
{
  setCategory(Hardware);
  setIconByName("printer");
  setStatus(Active);
}

PrintQueueTray::~PrintQueueTray()
{
}

void PrintQueueTray::connectToLauncher(const QString &destName)
{
    QSignalMapper *signalMapper = new QSignalMapper(this);
    signalMapper->setMapping(this, destName);
    connect(this, SIGNAL(activateRequested(bool, const QPoint &)), signalMapper, SLOT(map()));
    connect(signalMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(openQueue(const QString &)));
}

void PrintQueueTray::connectToMenu(const QList<QString> &printerList)
{
    QSignalMapper *signalMapper = new QSignalMapper(this);
    KMenu *printerMenu = new KMenu();

    foreach (const QString &printerName, printerList) {
        QAction *action = new QAction(KIcon("printer"),
                                      QString(i18n("Queue for ") + printerName + "..."), this);
        signalMapper->setMapping(action, printerName);
        connect(action, SIGNAL(triggered()), signalMapper, SLOT(map()));
        printerMenu->addAction(action);
    }

    connect(signalMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(openQueue(const QString &)));

    setAssociatedWidget(printerMenu);
}

void PrintQueueTray::openQueue(const QString &destName)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.PrintQueue",
                                             "/",
                                             "org.kde.PrintQueue",
                                             QLatin1String("ShowQueue"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(destName);
    QDBusConnection::sessionBus().call(message);
}
