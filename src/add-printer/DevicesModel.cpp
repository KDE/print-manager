/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DevicesModel.h"

#include <KCupsRequest.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QHostInfo>
#include <QDBusMetaType>
#include <QDBusConnection>

#include <QDebug>

DevicesModel::DevicesModel(QObject *parent) : QStandardItemModel(parent)
  , m_request(nullptr)
  , m_rx(QLatin1String("[a-z]+://.*"))
  , m_blacklistedURIs({
                      QLatin1String("hp"),
                      QLatin1String("hpfax"),
                      QLatin1String("hal"),
                      QLatin1String("beh"),
                      QLatin1String("scsi"),
                      QLatin1String("http"),
                      QLatin1String("delete")
                      })
{
    qDBusRegisterMetaType<MapSS>();
    qDBusRegisterMetaType<MapSMapSS>();

    // Adds the other device which is meant for manual URI input
    insertDevice(QLatin1String("other"),
                 QString(),
                 i18nc("@item", "Manual URI"),
                 QString(),
                 QLatin1String("other"),
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
    connect(m_request, &KCupsRequest::device, this, &DevicesModel::gotDevice);
    connect(m_request, &KCupsRequest::finished, this, &DevicesModel::finished);

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
    qDebug() << device_class;
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    qDebug() << device_id;
    // "Samsung SCX-4200 Series"
    qDebug() << device_info;
    // "Samsung SCX-4200 Series"
    qDebug() << device_make_and_model;
    // "usb://Samsung/SCX-4200%20Series"
    qDebug() << device_uri;
    // ""
    qDebug() << device_location;

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
        const MapSS mapSS({
                              {KCUPS_DEVICE_CLASS, device_class},
                              {KCUPS_DEVICE_ID, device_id},
                              {KCUPS_DEVICE_INFO, device_info},
                              {KCUPS_DEVICE_MAKE_AND_MODEL, device_make_and_model},
                              {KCUPS_DEVICE_LOCATION, device_location}
                          });
        m_mappedDevices[device_uri] = mapSS;
    }
}

void DevicesModel::finished()
{
    bool hasError = m_request->hasError();
    if (hasError) {
        Q_EMIT errorMessage(i18n("Failed to get a list of devices: '%1'", m_request->errorMsg()));
    }
    m_request->deleteLater();
    m_request = nullptr;

    if (hasError || m_mappedDevices.isEmpty()) {
        Q_EMIT loaded();
        return;
    }

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("/org/fedoraproject/Config/Printing"),
                                             QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("GroupPhysicalDevices"));
    message << QVariant::fromValue(m_mappedDevices);
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
        stdItem->setData(QVariant::fromValue(grouped_printers), DeviceUris);
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
    qDebug() << device_class;
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    qDebug() << device_id;
    // "Samsung SCX-4200 Series"
    qDebug() << device_info;
    // "Samsung SCX-4200 Series"
    qDebug() << device_make_and_model;
    // "usb://Samsung/SCX-4200%20Series"
    qDebug() << device_uri;
    // ""
    qDebug() << device_location;

    Kind kind;
    // Store the kind of the device
    if (device_class == QLatin1String("network")) {
        const auto match = m_rx.match(device_uri);
        if (match.hasMatch()) {
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
        text = device_info + QLatin1String(" (") + device_make_and_model + QLatin1Char(')');
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

    auto stdItem = new QStandardItem;
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
        catItem = findCreateCategory(i18nc("@item", "Discovered Network Printers"), kind);
        catItem->appendRow(stdItem);
        break;
    case OtherNetworked:
        catItem = findCreateCategory(i18nc("@item", "Other Network Printers"), kind);
        catItem->appendRow(stdItem);
        break;
    case Local:
        catItem = findCreateCategory(i18nc("@item", "Local Printers"), kind);
        catItem->appendRow(stdItem);
        break;
    default:
        stdItem->setData(kind, Qt::UserRole);
        appendRow(stdItem);
    }

    return stdItem;
}

void DevicesModel::getGroupedDevicesSuccess(const QDBusMessage &message)
{
    if (message.type() == QDBusMessage::ReplyMessage && message.arguments().size() == 1) {
        const auto argument = message.arguments().first().value<QDBusArgument>();
        const auto groupeDevices = qdbus_cast<QList<QStringList> >(argument);
        for (const QStringList &list : groupeDevices) {
            if (list.isEmpty()) {
                continue;
            }

            const QString uri = list.first();
            const MapSS device = m_mappedDevices[uri];
            insertDevice(device[KCUPS_DEVICE_CLASS],
                         device[KCUPS_DEVICE_ID],
                         device[KCUPS_DEVICE_INFO],
                         device[KCUPS_DEVICE_MAKE_AND_MODEL],
                         uri,
                         device[KCUPS_DEVICE_LOCATION],
                         list.size() > 1 ? list : QStringList());
        }
    } else {
        qWarning() << "Unexpected message" << message;
        groupedDevicesFallback();
    }
    Q_EMIT loaded();
}

void DevicesModel::getGroupedDevicesFailed(const QDBusError &error, const QDBusMessage &message)
{
    qWarning() << error <<  message;
    groupedDevicesFallback();
    Q_EMIT errorMessage(i18n("Failed to group devices: '%1'", error.message()));
    Q_EMIT loaded();
}

void DevicesModel::groupedDevicesFallback()
{
    MapSMapSS::const_iterator i = m_mappedDevices.constBegin();
    while (i != m_mappedDevices.constEnd()) {
        const MapSS device = i.value();
        insertDevice(device[KCUPS_DEVICE_CLASS],
                     device[KCUPS_DEVICE_ID],
                     device[KCUPS_DEVICE_INFO],
                     device[KCUPS_DEVICE_MAKE_AND_MODEL],
                     i.key(),
                     device[KCUPS_DEVICE_LOCATION]);
        ++i;
    }
}

QStandardItem* DevicesModel::findCreateCategory(const QString &category, Kind kind)
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *catItem = item(i);
        if (catItem->data(Qt::UserRole).toInt() == kind) {
            return catItem;
        }
    }

    int pos = 0;
    for (int i = 0; i < rowCount(); ++i, ++pos) {
        QStandardItem *catItem = item(i);
        if (catItem->data(Qt::UserRole).toInt() > kind) {
            pos = i;
            break;
        }
    }

    auto catItem = new QStandardItem(category);
    QFont font = catItem->font();
    font.setBold(true);
    catItem->setFont(font);
    catItem->setData(kind, Qt::UserRole);
    catItem->setFlags(Qt::ItemIsEnabled);
    insertRow(pos, catItem);

    // Emit the parent so the view expand the item
    Q_EMIT parentAdded(indexFromItem(catItem));

    return catItem;
}

#include "moc_DevicesModel.cpp"
