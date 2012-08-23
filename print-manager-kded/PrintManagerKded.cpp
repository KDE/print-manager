/***************************************************************************
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

#include "PrintManagerKded.h"

#include "NewPrinterNotification.h"

#include <QTimer>

#include <KGenericFactory>

K_PLUGIN_FACTORY(PrintDFactory, registerPlugin<PrintManagerKded>();)
K_EXPORT_PLUGIN(PrintDFactory("printmanager"))

PrintManagerKded::PrintManagerKded(QObject *parent, const QVariantList &args) :
    KDEDModule(parent),
    m_newPrinterNotification(0)
{
    Q_UNUSED(args)

    KGlobal::insertCatalog(QLatin1String("print-manager"));
    QTimer::singleShot(0, this, SLOT(loadThread()));
}

PrintManagerKded::~PrintManagerKded()
{
    if (m_newPrinterNotification) {
        m_newPrinterNotification->deleteLater();
    }
}

void PrintManagerKded::loadThread()
{
    m_newPrinterNotification = new NewPrinterNotification;
}
