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

#ifndef ADDPRINTER_H
#define ADDPRINTER_H

#include <QApplication>

class AddPrinterInterface;
class AddPrinter : public QApplication
{
    Q_OBJECT
public:
    AddPrinter(int &argc, char **argv);
    virtual ~AddPrinter();

    /**
     * This method allows to browse discovered printers and add them
     */
    void addPrinter(qulonglong wid);

    /**
     * This method allows to browse printers and create a class
     */
    void addClass(qulonglong wid);

    /**
     * This method allows to change the PPD of an existing printer
     */
    void changePPD(qulonglong wid, const QString &name);

    /**
     * This method allows to browse the PPD list,
     * and adding the printer described by device_id
     */
    void newPrinterFromDevice(qulonglong wid, const QString &name, const QString &device_id);

private:
    void show(QWidget *widget, qulonglong wid) const;

    AddPrinterInterface *m_pqInterface;
};

#endif //ADDPRINTER_H
