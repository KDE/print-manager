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

#ifndef PAGE_DESTINATIONS_H
#define PAGE_DESTINATIONS_H

#include "GenericPage.h"

#include <KPixmapSequenceOverlayPainter>

namespace Ui {
    class PageDestinations;
}

class DevicesModel;
class PageDestinations : public GenericPage
{
    Q_OBJECT
public:
    explicit PageDestinations(const QVariantHash &args = QVariantHash(), QWidget *parent = 0);
    ~PageDestinations();

    bool hasChanges() const;
    bool canProceed() const;
    void setValues(const QVariantHash &args);
    QVariantHash values() const;
    bool isValid() const;

private slots:
    void checkSelected();

private:
    Ui::PageDestinations *ui;
    KPixmapSequenceOverlayPainter *m_busySeq;
    DevicesModel *m_model;
    bool m_isValid;
};

#endif
