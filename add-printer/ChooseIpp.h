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

#ifndef CHOOSE_IPP_H
#define CHOOSE_IPP_H

#include "ui_ChooseIpp.h"

#include "GenericPage.h"
#include <QRegExp>

class ChooseIpp : public GenericPage, Ui::ChooseIpp
{
    Q_OBJECT
public:
    ChooseIpp(QWidget *parent = 0);
    ~ChooseIpp();

    void setValues(const QHash<QString, QVariant> &args);
    bool isValid() const;

public slots:
    void load();
    void on_detectPB_clicked();

private slots:
    void checkSelected();

private:
    bool m_isValid;
};

#endif
