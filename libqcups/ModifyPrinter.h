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

#ifndef MODIFY_PRINTER_H
#define MODIFY_PRINTER_H

#include "ui_ModifyPrinter.h"

#include "QCups.h"
#include <QWidget>

namespace QCups {

class ModifyPrinter : public QWidget, Ui::ModifyPrinter
{
    Q_OBJECT
public:
    explicit ModifyPrinter(const QString &destName, QWidget *parent = 0);
    ~ModifyPrinter();

public slots:
    void save();

private:
    Printer *m_printer;
};


}

#endif
