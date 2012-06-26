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

#ifndef PRINTER_PAGE_H
#define PRINTER_PAGE_H

#include <QWidget>
#include <KCupsPrinter.h>

class PrinterPage : public QWidget
{
    Q_OBJECT
public:
    PrinterPage(QWidget *parent = 0);
    virtual bool hasChanges() { return false; }

public:
    virtual void save() {}
    virtual QVariantHash modifiedValues() const;
    virtual QStringList neededValues() const { return QStringList(); }
    virtual void setRemote(bool remote);

signals:
    void changed(bool hasChanges);
};

#endif
