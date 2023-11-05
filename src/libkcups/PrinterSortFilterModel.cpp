/*
    SPDX-FileCopyrightText: 2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrinterSortFilterModel.h"

#include "kcupslib_log.h"
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
    Q_EMIT sourceModelChanged(model);
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
    Q_EMIT filteredPrintersChanged();
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
