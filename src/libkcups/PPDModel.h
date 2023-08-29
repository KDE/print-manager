/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PPD_MODEL_H
#define PPD_MODEL_H

#include <QStandardItemModel>
#include <kcupslib_export.h>

struct DriverMatch{
    QString ppd;
    QString match;
};

typedef QList<DriverMatch> DriverMatchList;

class KCUPSLIB_EXPORT PPDModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(Role)

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    typedef enum {
        PPDName = Qt::UserRole,
        PPDMake,
        PPDMakeAndModel
    } Role;

    explicit PPDModel(QObject *parent = nullptr);
    void setPPDs(const QList<QVariantMap> &ppds, const DriverMatchList &driverMatch = DriverMatchList());

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QHash<int,QByteArray> roleNames() const override;
    int count() const;

Q_SIGNALS:
    void countChanged();

private:
    QStandardItem* createPPDItem(const QVariantMap &ppd, bool recommended);
    QStandardItem* findCreateMake(const QString &make);

    QList<QVariantMap> m_ppds;
    QHash<int, QByteArray> m_roles;
};

Q_DECLARE_METATYPE(DriverMatchList)
Q_DECLARE_METATYPE(DriverMatch)

#endif
