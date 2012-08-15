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

#ifndef PAGE_CHOOSE_PRINTERS_H
#define PAGE_CHOOSE_PRINTERS_H

#include "GenericPage.h"

namespace Ui {
    class PageChoosePrinters;
}

class PageChoosePrinters : public GenericPage
{
    Q_OBJECT
public:
    PageChoosePrinters(const QVariantHash &args, QWidget *parent = 0);
    ~PageChoosePrinters();

    void setValues(const QVariantHash &args);
    QVariantHash values() const;
    bool canProceed() const;

private:
    Ui::PageChoosePrinters *ui;
};

#endif
