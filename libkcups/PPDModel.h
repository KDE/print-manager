/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PPD_MODEL_H
#define PPD_MODEL_H

#include <QStandardItemModel>
#include <QVariantHash>

struct DriverMatch{
    QString ppd;
    QString match;
};
typedef QList<DriverMatch> DriverMatchList;
class PPDModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(Role)
public:
    typedef enum {
        PPDName = Qt::UserRole,
        PPDMake,
        PPDMakeAndModel
    } Role;

    explicit PPDModel(QObject *parent = nullptr);
    void setPPDs(const QList<QVariantHash> &ppds, const DriverMatchList &driverMatch = DriverMatchList());

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void clear();

private:
    QStandardItem* createPPDItem(const QVariantHash &ppd, bool recommended);
    QStandardItem* findCreateMake(const QString &make);

    QList<QVariantHash> m_ppds;
};

Q_DECLARE_METATYPE(DriverMatchList)
Q_DECLARE_METATYPE(DriverMatch)

#endif
