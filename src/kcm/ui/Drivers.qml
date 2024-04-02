/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.plasma.components as PComp
import org.kde.kirigami as Kirigami
import org.kde.plasma.extras as PlasmaExtras

ColumnLayout {
    id: root
    spacing: Kirigami.Units.largeSpacing

    Layout.fillWidth: true
    Layout.fillHeight: true

    readonly property alias busy: kcmConn.loading

    // one of the recommended drivers was selected
    signal selected(var driver)
    // signal that no recommended drivers found, try fallback (manual)
    signal driverFallback()

    function load(devid, makeModel, uri) {
        kcmConn.loading = true
        kcm.clearRecommendedDrivers()
        kcm.getRecommendedDrivers(devid, makeModel, uri)
    }

    Component.onDestruction: kcm.clearRecommendedDrivers()

    // Fallback msg with the option to select the driver manually
    Kirigami.InlineMessage {
        id: fallbackMsg

        text: xi18nc("@info:status", "Unable to locate recommended drivers.  Click <interface>Refresh</interface> to try again or choose a driver manually.")
        showCloseButton: false
        Layout.fillWidth: true

        actions: [
            Kirigami.Action {
                icon.name: "document-edit-symbolic"
                text: i18nc("@action:button", "Choose Driver…")
                onTriggered: root.driverFallback()
            }
        ]
    }

    Connections {
        id: kcmConn
        target: kcm

        property bool loading: false

        function onRecommendedDriversLoaded() {
            kcmConn.loading = false

            if (recmlist.count === 0) {
                // If no drivers found, show fallback msg
                fallbackMsg.visible = true
            } else {
                let found = kcm.recommendedDrivers.findIndex(d => {return d.match === "exact"})
                if (found >= 0) {
                    recmlist.itemAtIndex(found).clicked()
                }
            }
        }
    }

    QQC2.Button {
        id: recmAction
        Layout.alignment: Qt.AlignHCenter
        visible: recmlist.count > 0
        icon.name: "dialog-ok-symbolic"
        text: i18nc("@action:button", "Select Recommended Driver")
        onClicked: {
            root.selected(kcm.recommendedDrivers[recmlist.currentIndex])
        }

        QQC2.ToolTip {
            text: i18nc("@info:tooltip", "Recommended drivers are based on printer make/model and connection type")
        }
    }

    QQC2.ScrollView {
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        Layout.fillHeight: true

        contentItem: ListView {
            id: recmlist

            PComp.BusyIndicator {
                running: kcmConn.loading
                anchors.centerIn: parent
                implicitWidth: Kirigami.Units.gridUnit * 6
                implicitHeight: Kirigami.Units.gridUnit * 6
            }

            activeFocusOnTab: true
            keyNavigationWraps: true

            KeyNavigation.backtab: root.parent
            Keys.onUpPressed: event => {
                if (currentIndex === 0) {
                    currentIndex = -1;
                }
                event.accepted = false;
            }

            model: kcm.recommendedDrivers

            delegate: Kirigami.SubtitleDelegate {
                width: ListView.view.width
                text: modelData["ppd-name"]
                subtitle: modelData.match
                icon.name: modelData.match.startsWith("exact")
                           ? "favorites-symbolic"
                           : "dialog-question-symbolic"

                highlighted: index === ListView.view.currentIndex
                onClicked: ListView.view.currentIndex = index
            }
        }
    }

}
