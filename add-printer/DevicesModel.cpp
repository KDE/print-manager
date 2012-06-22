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
   m_rx("[a-z]+://?")
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
}

void DevicesModel::update()
{
    if (m_request) {
        return;
    }

    // clear the model to don't duplicate items
    clear();
    m_request = new KCupsRequest;
    connect(m_request, SIGNAL(device(QString,QString,QString,QString,QString,QString)),
            this, SLOT(gotDevice(QString,QString,QString,QString,QString,QString)));
    connect(m_request, SIGNAL(finished()), this, SLOT(finished()));
    connect(m_request, SIGNAL(finished()), this, SIGNAL(loaded()));

    // Get devices with 5 seconds of timeout
    m_request->getDevices(5);

    // Adds the other device which is meant for manual URI input
    insertDevice(QString(),
                 QString(),
                 i18nc("@item", "Manual URI"),
                 QString(),
                 "other",
                 QString());
    gotDevice("direct",
              "MFG:Samsung;CMD:SPL,FWV,PIC,BDN,EXT;MDL:SCX-3400 Series;CLS:PRINTER;MODE:SCN,SPL3,R000105;STATUS:BUSY;",
              "Samsung SCX-3400 Series",
              "Samsung SCX-3400 Series",
              "usb://Samsung/SCX-3400%20Series?serial=Z6Y1BQAC500079K&interface=1",
              "");
    gotDevice("network",
              "MFG:Samsung;MDL:SCX-3400 Series;CMD:MFG:Samsung;CMD:SPL,FWV,PIC,BDN,EXT;MDL:SCX-3400 Series;CLS:PRINTER;MODE:SCN,SPL3,R000105;;",
              "Samsung SCX-3400 Series (SEC001599991856)",
              "Samsung SCX-3400 Series",
              "dnssd://Samsung%20SCX-3400%20Series%20(SEC001599991856)._ipp._tcp.local/",
              "");
    gotDevice("network",
              "MFG:Samsung;MDL:SCX-3400 Series;CMD:MFG:Samsung;CMD:SPL,FWV,PIC,BDN,EXT;MDL:SCX-3400 Series;CLS:PRINTER;MODE:SCN,SPL3,R000105;;",
              "Samsung SCX-3400 Series (SEC001599991856)",
              "Samsung SCX-3400 Series",
              "dnssd://Samsung%20SCX-3400%20Series%20(SEC001599991856)._pdl-datastream._tcp.local/",
              "");
    gotDevice("network",
              "MFG:Samsung;MDL:SCX-3400 Series;CMD:MFG:Samsung;CMD:SPL,FWV,PIC,BDN,EXT;MDL:SCX-3400 Series;CLS:PRINTER;MODE:SCN,SPL3,R000105;;",
              "Samsung SCX-3400 Series (SEC001599991856)",
              "Samsung SCX-3400 Series",
              "dnssd://Samsung%20SCX-3400%20Series%20(SEC001599991856)._printer._tcp.local/",
              "");
    gotDevice("network",
              "MFG:Samsung;CMD:SPL,FWV,PIC,BDN,EXT;MDL:SCX-3400 Series;CLS:PRINTER;MODE:SCN,SPL3,R000105;",
              "Samsung SCX-3400 Series",
              "Samsung SCX-3400 Series",
              "socket://192.168.1.141",
              "");
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
    m_request->deleteLater();
    m_request = 0;

    if (m_mappedDevices.isEmpty()) {
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
                                const QString &device_location, const QStringList &grouped_uris)
{
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
    } else {
        // If device class is not network assume local
        kind = Local;
    }
    kDebug() << device_class << kind << device_uri;

    QString location;
    if (device_location.isEmpty() && kind == Local) {
        location = QHostInfo::localHostName();
    } else {
        location = device_location;
    }

    QString text;
    if (!device_make_and_model.isEmpty() &&
            grouped_uris.isEmpty() &&
            device_make_and_model.compare(QLatin1String("unknown"), Qt::CaseInsensitive)) {
        text = device_info % QLatin1String(" (") % device_make_and_model % QLatin1Char(')');
    } else {
        text = device_info;
    }

    QString toolTip;
    if (grouped_uris.isEmpty()) {
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
    stdItem->setData(device_make_and_model, DeviceMakeAndModel);
    if (grouped_uris.isEmpty()) {
        stdItem->setData(device_uri, DeviceUris);
    } else {
        stdItem->setData(grouped_uris, DeviceUris);
    }
    stdItem->setData(device_location, DeviceLocation);

    // Find the proper category to our item
    QStandardItem *catItem;
    switch (kind) {
    case Networked:
        catItem = findCreateCategory(i18nc("@item", "Discovered Network Printers"));
        break;
    case OtherNetworked:
        catItem = findCreateCategory(i18nc("@item", "Other Network Printers"));
        break;
    default:
        catItem = findCreateCategory(i18nc("@item", "Local Printers"));
    }

    // Append the devie item to the row
    catItem->appendRow(stdItem);
}

void DevicesModel::getGroupedDevicesSuccess(const QDBusMessage &message)
{
    kDebug() << message;
    if (message.type() == QDBusMessage::ReplyMessage && message.arguments().size() == 1) {
        QDBusArgument argument = message.arguments().first().value<QDBusArgument>();
//        kDebug() << argument.asVariant();
        QList<QStringList> groupeDevices = qdbus_cast<QList<QStringList> >(argument);
        foreach (const QStringList &list, groupeDevices) {
            if (list.isEmpty()) {
                continue;
            }

            kDebug() << list.first() << m_mappedDevices[list.first()];
            MapSS device = m_mappedDevices[list.first()];
            insertDevice(device[KCUPS_DEVICE_CLASS],
                         device[KCUPS_DEVICE_ID],
                         device[KCUPS_DEVICE_INFO],
                         device[KCUPS_DEVICE_MAKE_AND_MODEL],
                         device[KCUPS_DEVICE_URI],
                         device[KCUPS_DEVICE_LOCATION],
                         list);
        }

//        m_driverMatchList = qdbus_cast<DriverMatchList>(argument);

//        foreach (const DriverMatch &driverMatch, m_driverMatchList) {
//            kDebug() << driverMatch.ppd << driverMatch.match;
//        }
    } else {
        kWarning() << "Unexpected message" << message;
    }
//    m_gotBestDrivers = true;
    //    setModelData();
}

void DevicesModel::getGroupedDevicesFailed(const QDBusError &error, const QDBusMessage &message)
{
    kDebug() << error <<  message;
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
    catItem->setSelectable(false);
    appendRow(catItem);

    // Emit the parent so the view expand the item
    emit parentAdded(indexFromItem(catItem));

    return catItem;
}
