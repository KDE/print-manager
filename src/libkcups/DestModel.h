/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEST_MODEL_H
#define DEST_MODEL_H

#include "IppBrowserManager.h"

#include <QStandardItemModel>

#include <qqmlregistration.h>

#include <kcupslib_export.h>

class KCupsRequest;

class KCUPSLIB_EXPORT DestModel : public QStandardItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Role {
        DeviceClass = Qt::UserRole,
        DeviceId,
        DeviceInfo,
        DeviceMakeAndModel,
        DeviceUri,
        DeviceUris,
        DeviceDescription,
        DeviceLocation
    };
    Q_ENUM(Role)

    explicit DestModel(QObject *parent = nullptr);
    virtual QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QString uriDevice(const QString &uri) const;
    Q_INVOKABLE void load();

Q_SIGNALS:
    void loaded();
    void errorMessage(const QString &message);

private:
    QString deviceDescription(const QString &uri) const;
    void createItem(const QVariantMap &device);
    void finished(KCupsRequest *request);

    QHash<int, QByteArray> m_roles;
    IppBrowserManager m_ippWatcher;
};

#endif
