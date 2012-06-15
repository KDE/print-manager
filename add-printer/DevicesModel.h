/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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
        DeviceLocation
    } Role;

    typedef enum {
        Local,
        Networked,
        OtherNetworked
    } Kind;

    DevicesModel(QObject *parent = 0);

signals:
    void loaded();
    void parentAdded(const QModelIndex &index);

public slots:
    void update();

private slots:
    void finished();
    void device(const QString &devClass,
                const QString &devId,
                const QString &devInfo,
                const QString &devMakeAndModel,
                const QString &devUri,
                const QString &devLocation);

private:
    QStandardItem *findCreateCategory(const QString &category);

    KCupsRequest *m_request;
    QRegExp m_rx;
};

#endif
