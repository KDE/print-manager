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

#include "PrintQueueSortFilterProxyModel.h"

#include "PrintQueueModel.h"

PrintQueueSortFilterProxyModel::PrintQueueSortFilterProxyModel(QObject *parent)
 : QSortFilterProxyModel(parent)
{
}

bool PrintQueueSortFilterProxyModel::lessThan(const QModelIndex &left,
                                              const QModelIndex &right) const
{
    if (left.column() == PrintQueueModel::ColStatus) {
        // the source model indices are sorted by the print queue
        // which has the right printing order.
        // The print order is about jobs creation and priority,
        // but we don't have to worry if we follow the getJobs order :D
        return left.row() < right.row();
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

#include "PrintQueueSortFilterProxyModel.moc"
