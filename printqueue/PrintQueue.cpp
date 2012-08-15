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

#include "PrintQueue.h"

#include "PrintQueueInterface.h"

#include <QTimer>

#include <KCmdLineArgs>
#include <KDebug>

PrintQueue::PrintQueue() :
    KUniqueApplication()
{
    m_pqInterface = new PrintQueueInterface(this);
    connect(m_pqInterface, SIGNAL(quit()), this, SLOT(quit()));
}

int PrintQueue::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QString queueName = args->getOption("show-queue");
    if (!queueName.isEmpty()) {
        m_pqInterface->ShowQueue(queueName);
    } else {
        // If DBus called the ui list won't be empty
        QTimer::singleShot(500, m_pqInterface, SLOT(RemoveQueue()));
    }

    return 0;
}

PrintQueue::~PrintQueue()
{
}

#include "PrintQueue.moc"
