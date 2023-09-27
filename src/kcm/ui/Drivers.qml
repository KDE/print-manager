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

    signal selected(var driver)

    function load(devid, makeModel, uri) {
        kcmConn.loading = true
        kcm.clearRecommendedDrivers()
        kcm.getRecommendedDrivers(devid, makeModel, uri)
    }

    Component.onDestruction: kcm.clearRecommendedDrivers()

    Connections {
        id: kcmConn
        target: kcm

        property bool loading: false

        function onRecommendedDriversLoaded() {
            kcmConn.loading = false
            let found = kcm.recommendedDrivers.findIndex(d => {return d.match === "exact"})
            if (found >= 0) {
                recmlist.itemAtIndex(found).onClicked()
            }
        }
    }

    QQC2.Button {
        id: recmAction
        Layout.alignment: Qt.AlignHCenter
        enabled: recmlist.count > 0
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
            highlight: PlasmaExtras.Highlight {}
            highlightMoveDuration: 0
            highlightResizeDuration: 0

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

                onClicked: {
                    ListView.view.currentIndex = index
                }
            }
        }
    }

}
