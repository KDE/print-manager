/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root
    spacing: Kirigami.Units.largeSpacing

    readonly property alias busy: kcmConn.loading

    // one of the recommended drivers was selected
    function selected(driverMap) {
        settings.set(driverMap)
        setValues(settings.pending)
        close()
    }

    function load(devid, makeModel, uri) {
        kcmConn.loading = true
        kcm.clearRecommendedDrivers()
        kcm.getRecommendedDrivers(devid, makeModel, uri)
    }

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
                onTriggered: manualDriverSelect()
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
            }
        }
    }

    QQC2.BusyIndicator {
        running: kcmConn.loading
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        implicitWidth: Kirigami.Units.gridUnit * 6
        implicitHeight: Kirigami.Units.gridUnit * 6
    }

    QQC2.Button {
        id: recmAction
        Layout.alignment: Qt.AlignHCenter
        visible: recmlist.count > 0
        icon.name: "dialog-ok-symbolic"
        text: i18nc("@action:button", "Select Recommended Driver")

        onClicked: selected(kcm.recommendedDrivers[recmlist.currentIndex])

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
