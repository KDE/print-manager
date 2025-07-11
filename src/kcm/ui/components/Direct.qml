/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * Device setup for directly connected printer devices,
 * ie. USB.
 *
 * The connection list shows the device connection options.
 * Each connection option has recommended drivers.
 * (system-config-printer)
*/
BaseDevice {
    id: root
    title: settings.value("printer-make-and-model")
    subtitle: settings.value("device-desc")
    helpText: uriModel ? i18nc("@info:usagetip", "Choose a device connection") : ""
    showUri: false

    readonly property var uriModel: settings.value("device-uris")

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        // Connection list
        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: directlist.count > 0

            Component.onCompleted: {
                if (background) {
                    background.visible = true
                }
            }

            ListView {
                id: directlist

                activeFocusOnTab: true
                keyNavigationWraps: true

                KeyNavigation.backtab: root.parent
                Keys.onUpPressed: event => {
                    if (currentIndex === 0) {
                        currentIndex = -1;
                    }
                    event.accepted = false;
                }

                model: uriModel

                delegate: QQC2.ItemDelegate {
                    width: ListView.view.width
                    text: devices.uriDevice(modelData)
                    icon.name: "standard-connector-symbolic"
                    highlighted: ListView.view.currentIndex === index

                    function getDrivers() : void {
                        ListView.view.currentIndex = index
                        settings.add("device-uri", modelData)
                        drivers.load(settings.value("device-id")
                                      , settings.value("printer-make-and-model")
                                      , modelData)
                    }

                    Component.onCompleted:  {
                        if (index === 0) {
                            getDrivers()
                        }
                    }

                    onClicked: getDrivers()
                }
            }
        }

        // Recommended Driver list
        Drivers {
            id: drivers
        }
    }
}

