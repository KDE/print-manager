/***************************************************************************
 *   Copyright (C) 2012-2013 by Daniel Nicoletti                           *
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
#include "JobSortFilterModel.h"

#include "JobModel.h"

#include <KDebug>

JobSortFilterModel::JobSortFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0);

    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SIGNAL(activeCountChanged()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SIGNAL(activeCountChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(activeCountChanged()));
    connect(this, SIGNAL(modelReset()),
            this, SIGNAL(activeCountChanged()));
    connect(this,  SIGNAL(activeCountChanged()), this, SLOT(syncRoleNames()));
}

void JobSortFilterModel::setModel(QAbstractItemModel *model)
{
    if (model == sourceModel()) {
        return;
    }

    if (sourceModel()) {
        disconnect(sourceModel(), SIGNAL(modelReset()), this, SLOT(syncRoleNames()));
    }

    QSortFilterProxyModel::setSourceModel(model);

    if (model) {
        connect(model, SIGNAL(modelReset()), this, SLOT(syncRoleNames()));
        syncRoleNames();
    }

    emit sourceModelChanged(model);
}

void JobSortFilterModel::setFilteredPrinters(const QString &printers)
{
    kDebug() << rowCount() << printers << printers.split(QLatin1Char('|'));
    if (printers.isEmpty()) {
        m_filteredPrinters.clear();
    } else {
        m_filteredPrinters = printers.split(QLatin1Char('|'));
    }
    invalidateFilter();
    emit filteredPrintersChanged();
}

void JobSortFilterModel::syncRoleNames()
{
    if (!sourceModel()) {
        return;
    }

    setRoleNames(sourceModel()->roleNames());
}

QString JobSortFilterModel::filteredPrinters() const
{
    return m_filteredPrinters.join(QLatin1String("|"));
}

int JobSortFilterModel::activeCount() const
{
    int active = 0;
    for (int i = 0; i < rowCount(); ++i) {
        QModelIndex item = index(i, 0);
        if (weightForState(item.data(JobModel::RoleJobState).toInt())) {
            ++active;
        }
    }
    return active;
}

bool JobSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // check if the printer is on the blacklist
    if (!m_filteredPrinters.isEmpty()) {
        return m_filteredPrinters.contains(index.data(JobModel::RoleJobPrinter).toString());
    }

    return true;
}

bool JobSortFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.column() == 0) {
        int leftWeight = weightForState(left.data(JobModel::RoleJobState).toInt());
        int rightWeight = weightForState(right.data(JobModel::RoleJobState).toInt());

        if (leftWeight != rightWeight) {
            return leftWeight > rightWeight;
        } else {
            int leftId = left.data(JobModel::RoleJobId).toInt();
            int rightId = right.data(JobModel::RoleJobId).toInt();

            if (leftWeight) {
                return leftId < rightId;
            }

            return leftId > rightId;
        }
    }
    return QSortFilterProxyModel::lessThan(left, right);
}

int JobSortFilterModel::weightForState(int state) const
{
    switch (state) {
    case IPP_JOB_PROCESSING:
        return 3;
        break;
    case IPP_JOB_PENDING:
        return 2;
        break;
    case IPP_JOB_HELD:
        return 1;
        break;
    default:
        return 0;
        break;
    }
}
