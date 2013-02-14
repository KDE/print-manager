/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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
    Q_ENUMS(Role)
public:
    typedef enum {
        DeviceClass = Qt::UserRole + 2,
        DeviceId,
        DeviceInfo,
        DeviceMakeAndModel,
        DeviceUri,
        DeviceUris,
        DeviceLocation
    } Role;

    typedef enum {
        Local,
        Networked,
        OtherNetworked,
        Other
    } Kind;

    DevicesModel(QObject *parent = 0);

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
    QStandardItem *findCreateCategory(const QString &category);

    KCupsRequest *m_request;
    MapSMapSS m_mappedDevices;
    QRegExp m_rx;
    QStringList m_blacklistedURIs;
};

Q_DECLARE_METATYPE(MapSS)
Q_DECLARE_METATYPE(MapSMapSS)

#endif
