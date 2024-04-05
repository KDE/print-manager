/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/*
 * This component is a catch-all for manually selected printers.
 * A host can be queried for printers it advertises, which will
 * display a list of "remote" printers. Additionally, a valid url
 * can be entered for a specific printer on the network.
*/
BaseDevice {
    id: uriItem

    title: compLoader.info
    subtitle: i18nc("@title:group", "Find remote printing devices")
    helpText: i18nc("@info:usagetip", "Enter the address of the remote host/device")
    icon.source: "internet-services"

    showAddressExamples: true
    addressExamples: [
        "ipp://ip-addr/ipp/print",
        "ipp://ip-addr-or-hostname/printers/name",
        "ipps://ip-addr/ipp/print",
        "ipps://ip-addr-or-hostname/printers/name",
        "http://ip-addr-or-hostname:port-number/printers/name"
    ]

    onUriSearch: uri => {
        if (uriItem.parseUri(uri)) {
           kcm.getRemotePrinters(uri, compLoader.selector)
        } else {
           kcm.clearRemotePrinters()
           list.model = ""
           setSearchPrefix()
        }
    }

    contentItem: ColumnLayout {
        width: uriItem.width
        spacing: Kirigami.Units.smallSpacing

        Component.onCompleted: setSearchPrefix()

        function setSearchPrefix() {
            uriItem.uriText = compLoader.selector !== "other"
                    ? compLoader.selector + ":"
                    : "ipp:"
        }

        // Remote printers load finished
        Connections {
            target: kcm

            function onRemotePrintersLoaded() {
                list.model = kcm.remotePrinters
            }
        }

        // Actions
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.smallSpacing

            QQC2.Button {
                text: i18nc("@action:button", "Select Printer…")
                visible: kcm.remotePrinters.length > 0
                icon.name: "dialog-ok-symbolic"
                onClicked: {
                    settings.set(kcm.remotePrinters[list.currentIndex])
                    manualDriverSelect()
                }
            }

            QQC2.Button {
                text: i18nc("@action:button", "Continue with Address…")
                icon.name: "dialog-ok-symbolic"
                onClicked: {
                    if (uriItem.parseUri(uriItem.uriText)) {
                        settings.add("device-uri", uriItem.uriText)
                        manualDriverSelect()
                    }
                }
            }
        }

        // remote printer list
        QQC2.ScrollView {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            contentItem: ListView {
                id: list
                currentIndex: -1
                clip: true

                activeFocusOnTab: true
                keyNavigationWraps: true

                KeyNavigation.backtab: uriItem.parent
                Keys.onUpPressed: event => {
                    if (currentIndex === 0) {
                        currentIndex = -1
                    }
                    event.accepted = false
                }

                delegate: Kirigami.SubtitleDelegate {
                    width: ListView.view.width
                    text: modelData["printer-info"]
                    subtitle: modelData["printer-name"]
                    icon.name: modelData.remote
                                ? "folder-network-symbolic"
                                : modelData["printer-is-class"] ? "folder" : modelData.iconName

                    Component.onCompleted: {
                        if (index === 0) {
                            clicked()
                        }
                    }

                    highlighted: ListView.view.currentIndex === index
                    onClicked: ListView.view.currentIndex = index
                }
            }
        }
    }
}
