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

#ifndef JOB_SORT_FILTER_MODEL_H
#define JOB_SORT_FILTER_MODEL_H

#include <QSortFilterProxyModel>

#include <QDeclarativeItem>

#include <kdemacros.h>

class KDE_EXPORT JobSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList filteredPrinters READ filteredPrinters WRITE setFilteredPrinters NOTIFY filteredPrintersChanged)
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit JobSortFilterModel(QObject *parent = 0);

    void setModel(QAbstractItemModel *model);
    void setFilteredPrinters(const QStringList &printers);
    QStringList filteredPrinters() const;
    int count() const;

signals:
    void countChanged();
    void sourceModelChanged(QObject *);
    void filteredPrintersChanged();

private slots:
    void syncRoleNames();

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    int weightForState(int state) const;

    QStringList m_filteredPrinters;
};

#endif // JOB_SORT_FILTER_MODEL_H
