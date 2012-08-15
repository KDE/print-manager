/***************************************************************************
 *   Copyright (C) 2010 Daniel Nicoletti <dantti12@gmail.com>              *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef ADD_PRINTER_INTERFACE_H
#define ADD_PRINTER_INTERFACE_H

#include <QtDBus/QDBusContext>

class QTimer;
class QWidget;
class AddPrinterInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.AddPrinter")
public:
    AddPrinterInterface(QObject *parent = 0);
    ~AddPrinterInterface();

    /**
     * This method allows to browse discovered printers and add them
     */
    void AddPrinter(qulonglong wid);

    /**
     * This method allows to browse printers and create a class
     */
    void AddClass(qulonglong wid);

    /**
     * This method allows to change the PPD of an existing printer
     */
    void ChangePPD(qulonglong wid, const QString &name);

    /**
     * This method allows to browse the PPD list,
     * and adding the printer described by device_id
     */
    void NewPrinterFromDevice(qulonglong wid, const QString &name, const QString &device_id);

signals:
    void quit();

private slots:
    void RemoveQueue();

private:
    void show(QWidget *widget, qulonglong wid) const;

    QTimer *m_updateUi;
    QHash<QString, QWidget *> m_uis;
};

#endif
