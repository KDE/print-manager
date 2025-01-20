/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DestModel.h"

#include <kcupslib_log.h>
#include <KCupsRequest.h>
#include <KDNSSD/ServiceBrowser>
#include <KLocalizedString>

using namespace Qt::StringLiterals;

const QString MANUAL = "manual"_L1;

DestModel::DestModel(QObject *parent)
    : QStandardItemModel(parent)
{
    m_roles[DeviceClass] = "deviceClass";
    m_roles[DeviceId] = "deviceId";
    m_roles[DeviceInfo] = "deviceInfo";
    m_roles[DeviceMakeAndModel] = "deviceMakeModel";
    m_roles[DeviceUri] = "deviceUri";
    m_roles[DeviceUris] = "deviceUris";
    m_roles[DeviceDescription] = "deviceDescription";
    m_roles[DeviceLocation] = "deviceLocation";

    m_ippWatcher.startBrowser(m_ippWatcher.defaultType());
}

QHash<int, QByteArray> DestModel::roleNames() const
{
    return m_roles;
}

QString DestModel::uriDevice(const QString &uri) const
{
    QString ret;
    if (uri.startsWith(QLatin1String("parallel"))) {
        ret = i18n("Parallel Port");
    } else if (uri.startsWith(QLatin1String("serial"))) {
        ret = i18n("Serial Port");
    } else if (uri.startsWith(QLatin1String("usb"))) {
        ret = i18n("USB");
    } else if (uri.startsWith(QLatin1String("bluetooth"))) {
        ret = i18n("Bluetooth");
    } else if (uri.startsWith(QLatin1String("hpfax"))) {
        ret = i18n("Fax - HP Linux Imaging and Printing (HPLIP)");
    } else if (uri.startsWith(QLatin1String("hp"))) {
        ret = i18n("HP Linux Imaging and Printing (HPLIP)");
    } else if (uri.startsWith(QLatin1String("hal"))) {
        ret = i18n("Hardware Abstraction Layer (HAL)");
    } else if (uri.startsWith(QLatin1String("socket"))) {
        ret = i18n("AppSocket/HP JetDirect");
    } else if (uri.startsWith(QLatin1String("lpd"))) {
        // Check if the queue name is defined
        const QString queue = uri.section(QLatin1Char('/'), -1, -1);
        if (queue.isEmpty()) {
            ret = i18n("LPD/LPR queue");
        } else {
            ret = i18n("LPD/LPR queue %1", queue);
        }
    } else if (uri.startsWith(QLatin1String("smb"))) {
        ret = i18n("Windows Printer via SAMBA");
    } else if (uri.startsWith(QLatin1String("ipp"))) {
        // Check if the queue name (fileName) is defined
        const QString queue = uri.section(QLatin1Char('/'), -1, -1);
        if (queue.isEmpty()) {
            ret = i18n("IPP");
        } else {
            ret = i18n("IPP %1", queue);
        }
    } else if (uri.startsWith(QLatin1String("https"))) {
        ret = i18n("HTTP");
    } else if (uri.startsWith(QLatin1String("dnssd")) || uri.startsWith(QLatin1String("mdns"))) {
        // TODO this needs testing...
        QString text;
        if (uri.contains(QLatin1String("cups"))) {
            text = i18n("Remote CUPS printer via DNS-SD");
        } else {
            if (uri.contains(QLatin1String("._ipp"))) {
                ret = i18n("IPP network printer via DNS-SD");
            } else if (uri.contains(QLatin1String("._printer"))) {
                ret = i18n("LPD network printer via DNS-SD");
            } else if (uri.contains(QLatin1String("._pdl-datastream"))) {
                ret = i18n("AppSocket/JetDirect network printer via DNS-SD");
            } else {
                ret = i18n("Network printer via DNS-SD");
            }
        }
    } else {
        ret = uri;
    }
    return ret;
}

QString DestModel::deviceDescription(const QString &uri) const
{
    static QMap<QString, QString> descriptions(
        {{u"parallel"_s, i18nc("@info:tooltip", "A printer connected to the parallel port")},
         {u"bluetooth"_s, i18nc("@info:tooltip", "A printer connected via Bluetooth")},
         {u"hal"_s, i18nc("@info:tooltip", "Local printer detected by the Hardware Abstraction Layer (HAL)")},
         {u"hpfax"_s, i18nc("@info:tooltip", "HPLIP software driving a fax machine,\nor the fax function of a multi-function device")},
         {u"hp"_s, i18nc("@info:tooltip", "HPLIP software driving a printer,\nor the printer function of a multi-function device")},
         {u"ipp"_s, i18nc("@info:tooltip", "IPP Network printer via IPP")},
         {u"usb"_s, i18nc("@info:tooltip", "A printer connected to a USB port")}});

    QString ret;

    if (uri.startsWith(u"dnssd"_s) || uri.startsWith(u"mdns"_s)) {
        if (uri.contains(u"cups"_s)) {
            ret = i18nc("@info:tooltip", "Remote CUPS printer via DNS-SD");
        } else {
            QString protocol;
            if (uri.contains(u"._ipp"_s)) {
                protocol = u"IPP"_s;
            } else if (uri.contains(u"._printer"_s)) {
                protocol = u"LPD"_s;
            } else if (uri.contains(u"._pdl-datastream"_s)) {
                protocol = u"AppSocket/JetDirect"_s;
            }

            if (protocol.isEmpty()) {
                ret = i18nc("@info:tooltip", "Network printer via DNS-SD");
            } else {
                ret = i18nc("@info:tooltip", "%1 network printer via DNS-SD", protocol);
            }
        }
    } else {
        for (auto [key, value] : descriptions.asKeyValueRange()) {
            if (uri.startsWith(key)) {
                return value;
            }
        }
    }

    return ret.isEmpty() ? uri : ret;
}

void DestModel::load()
{
    clear();
    const auto request = new KCupsRequest;
    connect(request, &KCupsRequest::deviceMap, this, &DestModel::createItem);
    connect(request, &KCupsRequest::finished, this, &DestModel::finished);
    request->getDestinations();

    // Manual URI input devices
    createItem({{KCUPS_DEVICE_CLASS, MANUAL}, {KCUPS_PRINTER_INFO, i18nc("@item", "Manual Config")}, {KCUPS_DEVICE_URI, "other"_L1}});
    createItem({{KCUPS_DEVICE_CLASS, MANUAL}, {KCUPS_PRINTER_INFO, uriDevice("dnssd._ipp"_L1)}, {KCUPS_DEVICE_URI, "ipp"_L1}});
    createItem({{KCUPS_DEVICE_CLASS, MANUAL}, {KCUPS_PRINTER_INFO, uriDevice("socket"_L1)}, {KCUPS_DEVICE_URI, "socket"_L1}});
    createItem({{KCUPS_DEVICE_CLASS, MANUAL}, {KCUPS_PRINTER_INFO, uriDevice("https"_L1)}, {KCUPS_DEVICE_URI, "http"_L1}});
    createItem({{KCUPS_DEVICE_CLASS, MANUAL}, {KCUPS_PRINTER_INFO, uriDevice("smb"_L1)}, {KCUPS_DEVICE_URI, "smb"_L1}});
}

void DestModel::finished(KCupsRequest *request)
{
    bool hasError = request->hasError();
    if (hasError) {
        qCDebug(LIBKCUPS) << "ERROR" << request->errorMsg();
        Q_EMIT errorMessage(i18n("Failed to get a list of devices: '%1'",request->errorMsg()));
    }

    request->deleteLater();
    Q_EMIT loaded();
}

void DestModel::createItem(const QVariantMap &device)
{
    const auto clas = device.value(KCUPS_DEVICE_CLASS).toString();
    const auto info = device.value(KCUPS_PRINTER_INFO).toString();
    const auto mm = device.value(KCUPS_PRINTER_MAKE_AND_MODEL).toString();
    const auto uri = device.value(KCUPS_DEVICE_URI).toString();
    const auto loc = device.value(KCUPS_PRINTER_LOCATION).toString();

    // FIXME? does this need to check all browers' services
    QString devid;
    if (clas != MANUAL) {
        devid = m_ippWatcher.getDeviceId(m_ippWatcher.defaultType(), info);
        if (devid.isEmpty()) {
            qCWarning(LIBKCUPS) << "UNABLE TO GET DEVID" << info;
            auto list = mm.split(" "_L1);
            const auto m = list.takeFirst();
            devid = QString("MFG:%1;MDL:%2"_L1).arg(m, list.join(" "_L1));
        }
    }

    // qCDebug(LIBKCUPS) << "DEST (class):" << clas;
    // qCDebug(LIBKCUPS) << devid;
    // qCDebug(LIBKCUPS) << info;
    // qCDebug(LIBKCUPS) << mm;
    // qCDebug(LIBKCUPS) << uri;

    const auto dd = deviceDescription(uri);
    auto stdItem = new QStandardItem;
    stdItem->setText(info);
    stdItem->setData(clas, DeviceClass);
    stdItem->setData(devid, DeviceId);
    stdItem->setData(info, DeviceInfo);
    stdItem->setData(mm, DeviceMakeAndModel);
    stdItem->setData(uri, DeviceUri);
    stdItem->setData(dd, DeviceDescription);
    stdItem->setData(loc, DeviceLocation);
    stdItem->setToolTip(dd);

    appendRow(stdItem);
}

#include "moc_DestModel.cpp"
