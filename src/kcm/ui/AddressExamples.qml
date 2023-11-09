/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.ScrollView {
    id: root
    signal selected(string address)

    property var examples: [
        "ipp://ip-addr/ipp/print",
        "ipp://ip-addr-or-hostname/printers/name",
        "ipps://ip-addr/ipp/print",
        "ipps://ip-addr-or-hostname/printers/name",
        "http://ip-addr-or-hostname:port-number/printers/name",
        "lpd://ip-addr/queue",
        "lpd://ip-addr/queue?format=l",
        "lpd://ip-addr/queue?format=l&reserve=rfc1179",
        "socket://ip-addr",
        "socket://ip-addr:port-number/?...",
        "socket://ip-addr/?contimeout=30",
        "socket://ip-addr/?waiteof=false",
        "socket://ip-addr/?contimeout=30&waiteof=false"
    ]

    contentItem: ListView {
        clip: true
        model: examples

        headerPositioning: ListView.OverlayHeader
        header: Kirigami.InlineViewHeader {
            text: i18nc("@info:usagetip", "Example Addresses")
            implicitWidth: ListView.view.width
        }

        delegate: QQC2.ItemDelegate {
            implicitWidth: ListView.view.width
            text: modelData
            onClicked: root.selected(modelData)
        }
    }
}
