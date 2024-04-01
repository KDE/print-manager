/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IPPBROWSER_H
#define IPPBROWSER_H

#include <KDNSSD/ServiceBrowser>

#include <kcupslib_export.h>

/**
 * @class IppBrowserManager
 * @short Manages the list of IPP browsers for printer services advertised over DNS-SD
 *
 * Declare the class and it will create a browser for service type "_ipp._tcp"
 * @code
 * IppBrowserManager m_Mgr
 * m_Mgr.startBrowser(m_Mgr.defaultType())
 * @endcode
 * - or -
 * Create an instance with
 * @code
 * ippB = new IppBrowserManager({"_ipps._tcp", "_http._tcp"}, this)
 * ippB.startBrowser("all")
 * @endcode
 *
 * @author Mike Noe
 */
class KCUPSLIB_EXPORT IppBrowserManager : public QObject
{
    Q_OBJECT

public:
    IppBrowserManager(QObject *parent = nullptr);
    IppBrowserManager(const QStringList &browseList, QObject *parent = nullptr);
    ~IppBrowserManager();

    KDNSSD::ServiceBrowser *createBrowser(const QString &type, bool startNow = true);
    void removeBrowser(const QString &type);
    KDNSSD::ServiceBrowser *getBrowser(const QString &type) const;

    /**
     * Return a constructed device-id from the ipp device
     * by @param ippServiceId.
     *
     * IPP Service Devices are indexed by CUPS printer-info.
     */
    QString getDeviceId(const QString &type, const QString &ippServiceId) const;

    QMap<QString, KDNSSD::ServiceBrowser *> browserList() const;

    void startBrowser(const QString &type = QLatin1String("all")) const;

    QString defaultType() const;

private:
    QMap<QString, KDNSSD::ServiceBrowser *> m_browserList;

Q_SIGNALS:
    void serviceAdded(KDNSSD::RemoteService::Ptr service);
    void serviceRemoved(KDNSSD::RemoteService::Ptr service);
    void finished();
};

#endif
