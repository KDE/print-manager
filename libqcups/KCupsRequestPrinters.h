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

#ifndef KCUPSREQUESTPRINTERS_H
#define KCUPSREQUESTPRINTERS_H

#include "KCupsRequestInterface.h"
#include "KCupsConnection.h"

class KDE_EXPORT KCupsRequestPrinters : public KCupsRequestInterface
{
    Q_OBJECT
public:
    explicit KCupsRequestPrinters();

    void setAttributes(const QString &destName, bool isClass, const Arguments &values, const char *filename = NULL);
    void setShared(const QString &destName, bool isClass, bool shared);
    void getAttributes(const QString &destName, bool isClass, const QStringList &requestedAttr);
    void printTestPage(const QString &destName, bool isClass);
    void printCommand(const QString &destName, const QString &command, const QString &title);
    KIcon icon(const QString &destName, int printerType);

signals:

public slots:

};

#endif // KCUPSREQUESTPRINTERS_H
