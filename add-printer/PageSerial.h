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

#ifndef PAGE_SERIAL_H
#define PAGE_SERIAL_H

#include "ui_PageSerial.h"

#include "GenericPage.h"
#include <QRegExp>

class PageSerial : public GenericPage, Ui::PageSerial
{
    Q_OBJECT
public:
    PageSerial(QWidget *parent = 0);
    ~PageSerial();

    void setValues(const QHash<QString, QString> &args);
    QHash<QString, QString> values();

public slots:
    void load();
    bool isValid();

private:
    QRegExp m_rx;
    bool m_isValid;
};

#endif
