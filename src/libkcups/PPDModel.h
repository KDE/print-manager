/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PPD_MODEL_H
#define PPD_MODEL_H

#include <qqmlregistration.h>
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
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    QML_ELEMENT

public:
    enum Role {
        PPDName = Qt::UserRole,
        PPDMake,
        PPDMakeAndModel
    };
    Q_ENUM(Role)

    explicit PPDModel(QObject *parent = nullptr);
    void setPPDs(const QList<QVariantMap> &ppds, const DriverMatchList &driverMatch = DriverMatchList());

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QHash<int,QByteArray> roleNames() const override;
    int count() const;

    Q_INVOKABLE void load();

Q_SIGNALS:
    void countChanged();
    void error(const QString &msg);

private:
    QStandardItem* createPPDItem(const QVariantMap &ppd, bool recommended);
    QStandardItem* findCreateMake(const QString &make);

    QList<QVariantMap> m_ppds;
    QHash<int, QByteArray> m_roles;
};

Q_DECLARE_METATYPE(DriverMatchList)
Q_DECLARE_METATYPE(DriverMatch)

#endif
