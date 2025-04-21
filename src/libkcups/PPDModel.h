/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023-2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PPD_MODEL_H
#define PPD_MODEL_H

#include <QStandardItemModel>
#include <kcupslib_export.h>
#include <qqmlregistration.h>

class KCUPSLIB_EXPORT PPDModel : public QStandardItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Role {
        PPDName = Qt::UserRole,
        PPDMake,
        PPDMakeAndModel
    };
    Q_ENUM(Role)

    explicit PPDModel(QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

Q_SIGNALS:
    void error(const QString &msg);
    void loaded();

private:
    QStandardItem *findCreateMake(const QString &make);

    QList<QVariantMap> m_ppds;
    QHash<int, QByteArray> m_roles;
};

#endif
