/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

pragma ComponentBehavior: Bound

/**
 * Device setup for Socket/JetDirect printer devices (Legacy)
*/
BaseDevice {
    id: socketDevice
    title: compLoader.info
    subtitle: i18nc("@title:group", "AppSocket Protocol (aka JetDirect)")
    helpText: i18nc("@info:usagetip", "Enter the address of the AppSocket device")
    showUriSearch: false

    scheme: compLoader.selector + "://"

    showAddressExamples: true
    addressExamples: [
        "socket://ip-addr",
        "socket://ip-addr:port-number/?...",
        "socket://ip-addr/?contimeout=30",
        "socket://ip-addr/?waiteof=false",
        "socket://ip-addr/?contimeout=30&waiteof=false"
    ]

    contentItem: ColumnLayout {
        Layout.preferredWidth: socketDevice.width
        // use large to match Base
        spacing: Kirigami.Units.largeSpacing

        Component.onCompleted: {
            const url = socketDevice.getUrl(settings.value("device-uri"))
            if (url) {
                socketDevice.uriText = url.href
            } else {
                socketDevice.uriText = socketDevice.scheme
            }
        }

        QQC2.Button {
            text: i18nc("@action:button", "Continue with Address…")
            icon.name: "dialog-ok-symbolic"
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                let uriStr = socketDevice.uriText
                if (!uriStr.startsWith(socketDevice.scheme)) {
                    uriStr = socketDevice.scheme + uriStr
                }
                // validate url
                const url = socketDevice.getUrl(uriStr)
                if (url) {
                    settings.set({"device-uri": url.href
                                 , "printer-info": "AppSocket Printer"})
                    manualDriverSelect()
                } else {
                    socketDevice.setError(i18n("Invalid Socket URL: %1", uriStr))
                }
            }
        }

        Item { Layout.fillHeight: true }

    }
}
