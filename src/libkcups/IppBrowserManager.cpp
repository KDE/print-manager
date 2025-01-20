/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "IppBrowserManager.h"

using namespace Qt::StringLiterals;

static constexpr QLatin1StringView DEFAULT_IPPTYPE{"_ipp._tcp"_L1};

IppBrowserManager::IppBrowserManager(QObject *parent)
    : QObject(parent)
{
    createBrowser(DEFAULT_IPPTYPE, false);
}

IppBrowserManager::IppBrowserManager(const QStringList &browseList, QObject *parent)
    : QObject(parent)
{
    Q_UNUSED(parent)

    for (const auto &type : browseList) {
        createBrowser(type);
    }
}

IppBrowserManager::~IppBrowserManager()
{
    for (const auto b : std::as_const(m_browserList)) {
        b->deleteLater();
    }
}

KDNSSD::ServiceBrowser *IppBrowserManager::createBrowser(const QString &type, bool startNow)
{
    auto sb = new KDNSSD::ServiceBrowser(type, true);
    if (sb) {
        m_browserList.insert(type, sb);

        connect(sb, &KDNSSD::ServiceBrowser::serviceAdded, this, &IppBrowserManager::serviceAdded);
        connect(sb, &KDNSSD::ServiceBrowser::serviceRemoved, this, &IppBrowserManager::serviceRemoved);
        connect(sb, &KDNSSD::ServiceBrowser::finished, this, &IppBrowserManager::finished);

        if (startNow) {
            sb->startBrowse();
        }
    }
    return sb;
}

void IppBrowserManager::removeBrowser(const QString &type)
{
    const auto b = m_browserList.take(type);
    if (b) {
        b->deleteLater();
    }
}

KDNSSD::ServiceBrowser *IppBrowserManager::getBrowser(const QString &type) const
{
    return m_browserList.value(type);
}

QString IppBrowserManager::getDeviceId(const QString &type, const QString &ippServiceId) const
{
    auto ippWatcher = getBrowser(type);
    if (!ippWatcher)
        return {};

    const auto servs = ippWatcher->services();
    if (const auto it = std::find_if(servs.begin(),
                                     servs.end(),
                                     [ippServiceId](const KDNSSD::RemoteService::Ptr &service) {
                                         return service->serviceName() == ippServiceId;
                                     });
        it != servs.end()) {
        const auto map = it->get()->textData();
        QStringList result;

        QString key;
        if (map.contains(u"usb_MFG"_s)) {
            key = u"usb_MFG"_s;
        } else if (map.contains(u"MFG"_s)) {
            key = u"MFG"_s;
        }
        if (!key.isEmpty()) {
            result << QString(u"MFG:"_s + QString::fromUtf8(map.value(key).constData()));
        }

        key.clear();
        if (map.contains(u"usb_MDL"_s)) {
            key = u"usb_MDL"_s;
        } else if (map.contains(u"MDL"_s)) {
            key = u"MDL"_s;
        }
        if (!key.isEmpty()) {
            result << QString(u"MDL:"_s + QString::fromUtf8(map.value(key).constData()));
        }

        // CMD: check for cmd first, then fallback to pdl
        if (map.contains(u"usb_CMD"_s)) {
            result << QString(u"CMD:"_s + QString::fromUtf8(map.value(u"usb_CMD"_s).constData()));
        } else if (map.contains(u"pdl"_s)) {
            const auto pdl = QString::fromUtf8(map.value(u"pdl"_s).constData());
            const auto list = pdl.split(u","_s);
            QStringList pdlList;
            for (const auto &s : list) {
                pdlList << s.split(u"/"_s)[1];
            }
            result << QString(u"CMD:"_s + pdlList.join(u","_s).toUpper());
        }

        return result.isEmpty() ? QString() : QString(result.join(u";"_s));
    }
    return {};
}

QMap<QString, KDNSSD::ServiceBrowser *> IppBrowserManager::browserList() const
{
    return m_browserList;
}

void IppBrowserManager::startBrowser(const QString &type) const
{
    if (type == "all"_L1) {
        for (const auto b : m_browserList) {
            b->startBrowse();
        }
        return;
    }

    if (const auto b = getBrowser(type)) {
        b->startBrowse();
    }
}

QString IppBrowserManager::defaultType() const
{
    return DEFAULT_IPPTYPE;
}
