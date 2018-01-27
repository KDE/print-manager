/***************************************************************************
 *   Copyright (C) 2010-2018 by Daniel Nicoletti                           *
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

#ifndef NEW_PRINTER_NOTIFICATION_H
#define NEW_PRINTER_NOTIFICATION_H

#include <QDBusContext>

class KNotification;
class NewPrinterNotification : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.redhat.NewPrinterNotification")
public:
    NewPrinterNotification(QObject *parent);
    ~NewPrinterNotification();

public:
    void GetReady();
    void NewPrinter(int status, const QString &name, const QString &make, const QString &model, const QString &des, const QString &cmd);

private:
    bool registerService();
    void configurePrinter();
    void printTestPage();
    void findDriver();

    void setupPrinterNotification(KNotification *notify, const QString &make, const QString &model, const QString &description, const QString &arg);
    void getMissingExecutables(KNotification *notify, int status, const QString &name, const QString &ppdFileName);
    void checkPrinterCurrentDriver(KNotification *notify, const QString &name);
    void printerReadyNotification(KNotification *notify, const QString &name);
};

#endif // NEW_PRINTER_NOTIFICATION_H
