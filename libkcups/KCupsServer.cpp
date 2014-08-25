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

#include "KCupsServer.h"

#include <cups/adminutil.h>
#include <cups/cups.h>

#include <config.h>

KCupsServer::KCupsServer()
{
}

KCupsServer::KCupsServer(const QVariantHash &arguments)
{
    m_arguments = arguments;
}

bool KCupsServer::allowRemoteAdmin() const
{
    return m_arguments.value(CUPS_SERVER_REMOTE_ADMIN).toBool();
}

void KCupsServer::setAllowRemoteAdmin(bool allow)
{
    m_arguments[CUPS_SERVER_REMOTE_ADMIN] = allow ? QLatin1String("1") : QLatin1String("0");
}

bool KCupsServer::allowUserCancelAnyJobs() const
{
    return m_arguments.value(CUPS_SERVER_USER_CANCEL_ANY).toBool();
}

void KCupsServer::setAllowUserCancelAnyJobs(bool allow)
{
    m_arguments[CUPS_SERVER_USER_CANCEL_ANY] = allow ? QLatin1String("1") : QLatin1String("0");
}

bool KCupsServer::showSharedPrinters() const
{
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    return m_arguments.value(CUPS_SERVER_REMOTE_PRINTERS).toBool();
#else
    return false;
#endif
}

void KCupsServer::setShowSharedPrinters(bool show)
{
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    m_arguments[CUPS_SERVER_REMOTE_PRINTERS] = show ? QLatin1String("1") : QLatin1String("0");
#else
    Q_UNUSED(show)
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
}

bool KCupsServer::sharePrinters() const
{
    return m_arguments.value(CUPS_SERVER_SHARE_PRINTERS).toBool();
}

void KCupsServer::setSharePrinters(bool share)
{
    m_arguments[CUPS_SERVER_SHARE_PRINTERS] = share ? QLatin1String("1") : QLatin1String("0");
}

bool KCupsServer::allowPrintingFromInternet() const
{
    return m_arguments.value(CUPS_SERVER_REMOTE_ANY).toBool();
}

void KCupsServer::setAllowPrintingFromInternet(bool allow)
{
    m_arguments[CUPS_SERVER_REMOTE_ANY] = allow ? QLatin1String("1") : QLatin1String("0");
}

QVariantHash KCupsServer::arguments() const
{
    return m_arguments;
}
