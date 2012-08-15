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

#ifndef CHOOSE_LPD_H
#define CHOOSE_LPD_H

#include "GenericPage.h"

namespace Ui {
    class ChooseLpd;
}
class ChooseLpd : public GenericPage
{
    Q_OBJECT
public:
    ChooseLpd(QWidget *parent = 0);
    ~ChooseLpd();

    void setValues(const QVariantHash &args);
    QVariantHash values() const;
    bool isValid() const;
    bool canProceed() const;

public slots:
    void on_addressLE_textChanged(const QString &text);

private slots:
    void checkSelected();

private:
    Ui::ChooseLpd *ui;
    bool m_isValid;
};

#endif
