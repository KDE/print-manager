/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
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

#include "NoSelectionRectDelegate.h"

NoSelectionRectDelegate::NoSelectionRectDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void NoSelectionRectDelegate::paint(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    // For some reason some styles don't honor the views SelectionRectVisible
    // and I just hate that selection rect thing...
    QStyleOptionViewItemV4 opt(option);
    if (opt.state & QStyle::State_HasFocus) {
        opt.state ^= QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter, opt, index);
}
