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

#include "DevicesModel.h"

#include <KCupsRequest.h>

#include <KLocale>
#include <KMessageBox>

#include <QHostInfo>
#include <QStringBuilder>
#include <QDBusMetaType>
#include <QDBusConnection>

#include <KDebug>

DevicesModel::DevicesModel(QObject *parent)
 : QStandardItemModel(parent),
   m_request(0),
   m_rx("[a-z]+://.*")
{
    qDBusRegisterMetaType<MapSS>();
    qDBusRegisterMetaType<MapSMapSS>();

    m_blacklistedURIs << QLatin1String("hp");
    m_blacklistedURIs << QLatin1String("hpfax");
    m_blacklistedURIs << QLatin1String("hal");
    m_blacklistedURIs << QLatin1String("beh");
    m_blacklistedURIs << QLatin1String("scsi");
    m_blacklistedURIs << QLatin1String("http");
    m_blacklistedURIs << QLatin1String("delete");

    // Adds the other device which is meant for manual URI input
    insertDevice("other",
                 QString(),
                 i18nc("@item", "Manual URI"),
                 QString(),
                 "other",
                 QString());
}

void DevicesModel::update()
{
    if (m_request) {
        return;
    }

    // clear the model to don't duplicate items
    if (rowCount()) {
        removeRows(1, rowCount() - 1);
    }
    m_request = new KCupsRequest;
    connect(m_request, SIGNAL(device(QString,QString,QString,QString,QString,QString)),
            this, SLOT(gotDevice(QString,QString,QString,QString,QString,QString)));
    connect(m_request, SIGNAL(finished()), this, SLOT(finished()));

    // Get devices with 5 seconds of timeout
    m_request->getDevices(10);
}


void DevicesModel::gotDevice(const QString &device_class,
                             const QString &device_id,
                             const QString &device_info,
                             const QString &device_make_and_model,
                             const QString &device_uri,
                             const QString &device_location)
{
    // "direct"
    kDebug() << device_class;
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    kDebug() << device_id;
    // "Samsung SCX-4200 Series"
    kDebug() << device_info;
    // "Samsung SCX-4200 Series"
    kDebug() << device_make_and_model;
    // "usb://Samsung/SCX-4200%20Series"
    kDebug() << device_uri;
    // ""
    kDebug() << device_location;

    if (m_blacklistedURIs.contains(device_uri)) {
        // ignore black listed uri's
        return;
    }

    // For the protocols, not real devices
    if (device_id.isEmpty() &&
            device_make_and_model == QLatin1String("Unknown")) {
        insertDevice(device_class,
                     device_id,
                     device_info,
                     device_make_and_model,
                     device_uri,
                     device_location);
    } else {
        // Map the devices so later we try to group them
        MapSS mapSS;
        mapSS[KCUPS_DEVICE_CLASS] = device_class;
        mapSS[KCUPS_DEVICE_ID] = device_id;
        mapSS[KCUPS_DEVICE_INFO] = device_info;
        mapSS[KCUPS_DEVICE_MAKE_AND_MODEL] = device_make_and_model;
        mapSS[KCUPS_DEVICE_LOCATION] = device_location;
        m_mappedDevices[device_uri] = mapSS;
    }
}

void DevicesModel::finished()
{
    bool hasError = m_request->hasError();
    if (hasError) {
        emit errorMessage(i18n("Failed to get a list of devices: '%1'", m_request->errorMsg()));
    }
    m_request->deleteLater();
    m_request = 0;

    if (hasError || m_mappedDevices.isEmpty()) {
        emit loaded();
        return;
    }

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("/org/fedoraproject/Config/Printing"),
                                             QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("GroupPhysicalDevices"));
    message << qVariantFromValue(m_mappedDevices);
    QDBusConnection::sessionBus().callWithCallback(message,
                                                   this,
                                                   SLOT(getGroupedDevicesSuccess(QDBusMessage)),
                                                   SLOT(getGroupedDevicesFailed(QDBusError,QDBusMessage)));
}

void DevicesModel::insertDevice(const QString &device_class,
                                const QString &device_id,
                                const QString &device_info,
                                const QString &device_make_and_model,
                                const QString &device_uri,
                                const QString &device_location,
                                const QStringList &grouped_uris)
{
    QStandardItem *stdItem;
    stdItem = createItem(device_class,
                         device_id,
                         device_info,
                         device_make_and_model,
                         device_uri,
                         device_location,
                         !grouped_uris.isEmpty());
    if (!grouped_uris.isEmpty()) {
        stdItem->setData(grouped_uris, DeviceUris);
    }
}

void DevicesModel::insertDevice(const QString &device_class,
                                const QString &device_id,
                                const QString &device_info,
                                const QString &device_make_and_model,
                                const QString &device_uri,
                                const QString &device_location,
                                const KCupsPrinters &grouped_printers)
{
    QStandardItem *stdItem;
    stdItem = createItem(device_class,
                         device_id,
                         device_info,
                         device_make_and_model,
                         device_uri,
                         device_location,
                         !grouped_printers.isEmpty());
    if (!grouped_printers.isEmpty()) {
        stdItem->setData(qVariantFromValue(grouped_printers), DeviceUris);
    }
}

QStandardItem *DevicesModel::createItem(const QString &device_class,
                                        const QString &device_id,
                                        const QString &device_info,
                                        const QString &device_make_and_model,
                                        const QString &device_uri,
                                        const QString &device_location,
                                        bool grouped)
{
    // "direct"
    kDebug() << device_class;
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    kDebug() << device_id;
    // "Samsung SCX-4200 Series"
    kDebug() << device_info;
    // "Samsung SCX-4200 Series"
    kDebug() << device_make_and_model;
    // "usb://Samsung/SCX-4200%20Series"
    kDebug() << device_uri;
    // ""
    kDebug() << device_location;

    Kind kind;
    // Store the kind of the device
    if (device_class == QLatin1String("network")) {
        if (m_rx.indexIn(device_uri) > -1) {
            kind = Networked;
        } else {
            // other network devices looks like
            // just "http"
            kind = OtherNetworked;
        }
    } else if (device_class == QLatin1String("other") &&
               device_uri == QLatin1String("other")) {
        kind = Other;
    } else {
        // If device class is not network assume local
        kind = Local;
    }

    QString location;
    if (device_location.isEmpty() && kind == Local) {
        location = QHostInfo::localHostName();
    } else {
        location = device_location;
    }

    QString text;
    if (!device_make_and_model.isEmpty() &&
            !grouped &&
            device_make_and_model.compare(QLatin1String("unknown"), Qt::CaseInsensitive)) {
        text = device_info % QLatin1String(" (") % device_make_and_model % QLatin1Char(')');
    } else {
        text = device_info;
    }

    QString toolTip;
    if (!grouped) {
        if (device_uri.startsWith(QLatin1String("parallel"))) {
            toolTip = i18nc("@info:tooltip",
                            "A printer connected to the parallel port");
        } else if (device_uri.startsWith(QLatin1String("usb"))) {
            toolTip = i18nc("@info:tooltip",
                            "A printer connected to a USB port");
        } else if (device_uri.startsWith(QLatin1String("bluetooth"))) {
            toolTip = i18nc("@info:tooltip",
                            "A printer connected via Bluetooth");
        } else if (device_uri.startsWith(QLatin1String("hal"))) {
            toolTip = i18nc("@info:tooltip",
                            "Local printer detected by the "
                            "Hardware Abstraction Layer (HAL)");
        } else if (device_uri.startsWith(QLatin1String("hp"))) {
            toolTip = i18nc("@info:tooltip",
                            "HPLIP software driving a printer, "
                            "or the printer function of a multi-function device");
        } else if (device_uri.startsWith(QLatin1String("hpfax"))) {
            toolTip = i18nc("@info:tooltip",
                            "HPLIP software driving a fax machine, "
                            "or the fax function of a multi-function device");
        } else if (device_uri.startsWith(QLatin1String("dnssd")) ||
                   device_uri.startsWith(QLatin1String("mdns"))) {
            toolTip = i18nc("@info:tooltip",
                            "Remote CUPS printer via DNS-SD");
        }
    }

    QStandardItem *stdItem = new QStandardItem;
    stdItem->setText(text);
    stdItem->setToolTip(toolTip);
    stdItem->setData(device_class, DeviceClass);
    stdItem->setData(device_id, DeviceId);
    stdItem->setData(device_info, DeviceInfo);
    stdItem->setData(device_uri, DeviceUri);
    stdItem->setData(device_make_and_model, DeviceMakeAndModel);
    stdItem->setData(device_location, DeviceLocation);

    // Find the proper category to our item
    QStandardItem *catItem;
    switch (kind) {
    case Networked:
        catItem = findCreateCategory(i18nc("@item", "Discovered Network Printers"));
        catItem->appendRow(stdItem);
        break;
    case OtherNetworked:
        catItem = findCreateCategory(i18nc("@item", "Other Network Printers"));
        catItem->appendRow(stdItem);
        break;
    case Local:
        catItem = findCreateCategory(i18nc("@item", "Local Printers"));
        catItem->appendRow(stdItem);
        break;
    default:
        appendRow(stdItem);
    }

    return stdItem;
}

void DevicesModel::getGroupedDevicesSuccess(const QDBusMessage &message)
{
    if (message.type() == QDBusMessage::ReplyMessage && message.arguments().size() == 1) {
        QDBusArgument argument;
        argument = message.arguments().first().value<QDBusArgument>();
        QList<QStringList> groupeDevices;
        groupeDevices = qdbus_cast<QList<QStringList> >(argument);
        foreach (const QStringList &list, groupeDevices) {
            if (list.isEmpty()) {
                continue;
            }

            QString uri = list.first();
            MapSS device = m_mappedDevices[uri];
            insertDevice(device[KCUPS_DEVICE_CLASS],
                         device[KCUPS_DEVICE_ID],
                         device[KCUPS_DEVICE_INFO],
                         device[KCUPS_DEVICE_MAKE_AND_MODEL],
                         uri,
                         device[KCUPS_DEVICE_LOCATION],
                         list.size() > 1 ? list : QStringList());
        }
    } else {
        kWarning() << "Unexpected message" << message;
        groupedDevicesFallback();
    }
    emit loaded();
}

void DevicesModel::getGroupedDevicesFailed(const QDBusError &error, const QDBusMessage &message)
{
    kWarning() << error <<  message;
    groupedDevicesFallback();
    emit errorMessage(i18n("Failed to group devices: '%1'",error.message()));
    emit loaded();
}

void DevicesModel::groupedDevicesFallback()
{
    MapSMapSS::const_iterator i = m_mappedDevices.constBegin();
    while (i != m_mappedDevices.constEnd()) {
        MapSS device = i.value();
        insertDevice(device[KCUPS_DEVICE_CLASS],
                     device[KCUPS_DEVICE_ID],
                     device[KCUPS_DEVICE_INFO],
                     device[KCUPS_DEVICE_MAKE_AND_MODEL],
                     i.key(),
                     device[KCUPS_DEVICE_LOCATION]);
        ++i;
    }
}

QStandardItem* DevicesModel::findCreateCategory(const QString &category)
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *catItem = item(i);
        if (catItem->text() == category) {
            return catItem;
        }
    }

    QStandardItem *catItem = new QStandardItem(category);
    QFont font = catItem->font();
    font.setBold(true);
    catItem->setFont(font);
    catItem->setFlags(Qt::ItemIsEnabled);
    appendRow(catItem);

    // Emit the parent so the view expand the item
    emit parentAdded(indexFromItem(catItem));

    return catItem;
}
