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

#include <QCups.h>
#include <cups/cups.h>

#include <KCategorizedSortFilterProxyModel>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>

DevicesModel::DevicesModel(QObject *parent)
 : QStandardItemModel(parent),
   m_ret(0),
   m_rx("[a-z]+://?")
{
}

void DevicesModel::update()
{
    if (m_ret) {
        return;
    }

    // clear the model to don't duplicate items
    clear();
    m_ret = QCups::getDevices();
    connect(m_ret, SIGNAL(device(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &)),
            this, SLOT(device(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &)));
    connect(m_ret, SIGNAL(finished()), this, SLOT(finished()));
    connect(m_ret, SIGNAL(finished()), this, SIGNAL(loaded()));
}

void DevicesModel::device(const QString &devClass,
                          const QString &devId,
                          const QString &devInfo,
                          const QString &devMakeAndModel,
                          const QString &devUri,
                          const QString &devLocation)
{
    Q_UNUSED(devId)
    Q_UNUSED(devLocation)
    // Example of data
    // "direct"
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    // "Samsung SCX-4200 Series"
    // "Samsung SCX-4200 Series"
    // "usb://Samsung/SCX-4200%20Series"
    // ""

    Kind kind;
    // Store the kind of the device
    if (devClass == "network") {
        if (m_rx.indexIn(devUri) > -1) {
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

//     QStandardItem *itemClass;
//     int parentRow = classRow(kind); // Check if there is a parent row already
//     if (parentRow != -1) {
//         itemClass = item(parentRow); // if so get it's item''
//     } else {
//         if (kind == Networked) {
//             itemClass = new QStandardItem(i18n("Discovered Network Printers"));
//         } else if (kind == OtherNetworked) {
//             itemClass = new QStandardItem(i18n("Other Network Printers"));
//         } else {
//             itemClass = new QStandardItem(i18n("Local Printers"));
//         }
//         // store the data to check later
//         itemClass->setData(kind);
//         // do not allow the user to select the root item
//         // to not enable the next button
//         itemClass->setSelectable(false);
//         QFont font = itemClass->font();
//         font.setPointSize(font.pointSize() + 2);
//         itemClass->setFont(font);
//         appendRow(itemClass);
//     }

    QStandardItem *device;
    if (devMakeAndModel != "Unknown") {
        device = new QStandardItem(devInfo + " (" + devMakeAndModel + ')');
        device->setData(devUri, DeviceURI);
        device->setData(devMakeAndModel, DeviceMakeAndModel);
    } else {
        device = new QStandardItem(devInfo);
        device->setData(devUri, DeviceURI);
    }

    if (devUri.startsWith(QLatin1String("parallel"))) {
        device->setToolTip(i18nc("@info:tooltip", "A printer connected to the parallel port"));
    } else if (devUri.startsWith(QLatin1String("usb"))) {
        device->setToolTip(i18nc("@info:tooltip", "A printer connected to a USB port"));
    } else if (devUri.startsWith(QLatin1String("bluetooth"))) {
        device->setToolTip(i18nc("@info:tooltip", "A printer connected via Bluetooth"));
    } else if (devUri.startsWith(QLatin1String("hal"))) {
        device->setToolTip(i18nc("@info:tooltip", "Local printer detected by the "
                                "Hardware Abstraction Layer (HAL)"));
    } else if (devUri.startsWith(QLatin1String("hp"))) {
        device->setToolTip(i18nc("@info:tooltip", "HPLIP software driving a printer, "
                                "or the printer function of a multi-function device"));
    } else if (devUri.startsWith(QLatin1String("hpfax"))) {
        device->setToolTip(i18nc("@info:tooltip", "HPLIP software driving a fax machine, "
                                "or the fax function of a multi-function device"));
    } else if (devUri.startsWith(QLatin1String("dnssd")) ||
               devUri.startsWith(QLatin1String("mdns"))) {
        device->setToolTip(i18nc("@info:tooltip", "Remote CUPS printer via DNS-SD"));
    }
    device->setData(devInfo, DeviceInfo);

    device->setData(static_cast<qlonglong>(kind), KCategorizedSortFilterProxyModel::CategorySortRole);
    if (kind == Networked) {
        device->setData(i18nc("@item", "Discovered Network Printers"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    } else if (kind == OtherNetworked) {
        device->setData(i18nc("@item", "Other Network Printers"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    } else {
        device->setData(i18nc("@item", "Local Printers"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    }
    appendRow(device);
//     itemClass->appendRow(device);
//     kDebug() << devClass << devId << devInfo << devMakeAndModel << devUri << devLocation;
}

void DevicesModel::finished()
{
    m_ret->deleteLater();
    m_ret = 0;
}


#include "DevicesModel.moc"
