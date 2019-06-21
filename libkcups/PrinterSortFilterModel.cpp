/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti                                *
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
#include "PrinterSortFilterModel.h"

#include "Debug.h"
#include "PrinterModel.h"

PrinterSortFilterModel::PrinterSortFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    sort(0);

    connect(this, &PrinterSortFilterModel::rowsInserted, this, &PrinterSortFilterModel::countChanged);
    connect(this, &PrinterSortFilterModel::rowsRemoved, this, &PrinterSortFilterModel::countChanged);
    connect(this, &PrinterSortFilterModel::modelReset, this, &PrinterSortFilterModel::countChanged);
}

void PrinterSortFilterModel::setModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }

    QSortFilterProxyModel::setSourceModel(model);
    emit sourceModelChanged(model);
}

void PrinterSortFilterModel::setFilteredPrinters(const QString &printers)
{
    qCDebug(LIBKCUPS) << rowCount() << printers << printers.split(QLatin1Char('|'));
    if (printers.isEmpty()) {
        m_filteredPrinters.clear();
    } else {
        m_filteredPrinters = printers.split(QLatin1Char('|'));
    }
    invalidateFilter();
    emit filteredPrintersChanged();
}

QString PrinterSortFilterModel::filteredPrinters() const
{
    return m_filteredPrinters.join(QLatin1String("|"));
}

int PrinterSortFilterModel::count() const
{
    return rowCount();
}

bool PrinterSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // check if the printer is on the blacklist
    if (!m_filteredPrinters.isEmpty()) {
        return m_filteredPrinters.contains(index.data(PrinterModel::DestName).toString());
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

//bool PrinterSortFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
//{
//    bool leftIsRemote = sourceModel()->data(left, PrinterModel::DestRemote).toBool();
//    bool rightIsRemote = sourceModel()->data(right, PrinterModel::DestRemote).toBool();
//    bool leftDefault = sourceModel()->data(left, PrinterModel::DestIsDefault).toBool();
//    bool rightDefault = sourceModel()->data(right, PrinterModel::DestIsDefault).toBool();

//    if (leftDefault != rightDefault) {
//        return leftDefault;
//    }

//    if (leftIsRemote != rightIsRemote) {
//        // If the right item is a remote the left should move right
//        return rightIsRemote;
//    }

//    return QSortFilterProxyModel::lessThan(left, right);
//}

#include "moc_PrinterSortFilterModel.cpp"
