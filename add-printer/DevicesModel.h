/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEVICES_MODEL_H
#define DEVICES_MODEL_H

#include <QStandardItemModel>
#include <QDBusMessage>

#include <KCupsPrinter.h>

typedef QMap<QString, QString> MapSS;
typedef QMap<QString, MapSS> MapSMapSS;

class KCupsRequest;
class DevicesModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum Role {
        DeviceClass = Qt::UserRole + 2,
        DeviceId,
        DeviceInfo,
        DeviceMakeAndModel,
        DeviceUri,
        DeviceUris,
        DeviceLocation
    };
    Q_ENUM(Role)

    enum Kind {
        Other,
        Local,
        Networked,
        OtherNetworked
    };
    Q_ENUM(Kind)

    explicit DevicesModel(QObject *parent = nullptr);

signals:
    void loaded();
    void parentAdded(const QModelIndex &index);
    void errorMessage(const QString &message);

public slots:
    void update();
    void insertDevice(const QString &device_class,
                      const QString &device_id,
                      const QString &device_info,
                      const QString &device_make_and_model,
                      const QString &device_uri,
                      const QString &device_location,
                      const QStringList &grouped_uris = QStringList());
    void insertDevice(const QString &device_class,
                      const QString &device_id,
                      const QString &device_info,
                      const QString &device_make_and_model,
                      const QString &device_uri,
                      const QString &device_location,
                      const KCupsPrinters &grouped_printers);

private slots:
    QStandardItem* createItem(const QString &device_class,
                              const QString &device_id,
                              const QString &device_info,
                              const QString &device_make_and_model,
                              const QString &device_uri,
                              const QString &device_location,
                              bool grouped);
    void gotDevice(const QString &device_class,
                   const QString &device_id,
                   const QString &device_info,
                   const QString &device_make_and_model,
                   const QString &device_uri,
                   const QString &device_location);
    void finished();
    void getGroupedDevicesSuccess(const QDBusMessage &message);
    void getGroupedDevicesFailed(const QDBusError &error, const QDBusMessage &message);
    void groupedDevicesFallback();

private:
    QStandardItem *findCreateCategory(const QString &category, Kind kind);

    KCupsRequest *m_request;
    MapSMapSS m_mappedDevices;
    QRegExp m_rx;
    QStringList m_blacklistedURIs;
};

Q_DECLARE_METATYPE(MapSS)
Q_DECLARE_METATYPE(MapSMapSS)

#endif
