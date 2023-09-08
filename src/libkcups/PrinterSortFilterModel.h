/*
    SPDX-FileCopyrightText: 2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTERSORTFILTERMODEL_H
#define PRINTERSORTFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <qqmlregistration.h>
#include <kcupslib_export.h>

class KCUPSLIB_EXPORT PrinterSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString filteredPrinters READ filteredPrinters WRITE setFilteredPrinters NOTIFY filteredPrintersChanged)
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setModel NOTIFY sourceModelChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit PrinterSortFilterModel(QObject *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    void setFilteredPrinters(const QString &printers);
    QString filteredPrinters() const;
    int count() const;

signals:
    void countChanged();
    void sourceModelChanged(QObject *);
    void filteredPrintersChanged();

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
//    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    QStringList m_filteredPrinters;
};

#endif // PRINTERSORTFILTERMODEL_H
