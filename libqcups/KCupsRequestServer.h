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

#ifndef KCUPSREQUESTSERVER_H
#define KCUPSREQUESTSERVER_H

#include "KCupsRequestInterface.h"

class KCupsRequestServer : public KCupsRequestInterface
{
    Q_OBJECT
public:
    explicit KCupsRequestServer(QObject *parent = 0);

    void adminSetServerSettings(const HashStrStr &userValues);
    void getPPDS(const QString &make = QString());

    void getDevices();
    // THIS function can get the default server dest through the
    // "printer-is-default" attribute BUT it does not get user
    // defined default printer, see cupsGetDefault() on www.cups.org for details
    void getDests(int mask, const QStringList &requestedAttr = QStringList());
    void getJobs(const QString &destName, bool myJobs, int whichJobs, const QStringList &requestedAttr = QStringList());

    void addClass(const QHash<QString, QVariant> &values);

    /*
     The result will be in hashStrStr()
    */
    void adminGetServerSettings();
};

#endif // KCUPSREQUESTSERVER_H
