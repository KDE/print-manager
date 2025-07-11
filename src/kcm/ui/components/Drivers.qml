/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PM

ColumnLayout {
    id: root
    spacing: Kirigami.Units.smallSpacing

    readonly property bool ippCapable: kcm.isIPPCapable(settings.value("device-uri"))
    readonly property alias busy: kcmConn.loading

    // one of the recommended drivers was selected
    function selectDriver(driverMap) {
        settings.set(driverMap)
        setValues(settings.pending)
    }

    function load(devid, makeModel, uri) {
        if (busy) {
            return
        }

        kcmConn.loading = true
        kcm.getRecommendedDrivers(devid, makeModel, uri)
    }

    Connections {
        id: kcmConn
        target: kcm

        property bool loading: false

        function onRecommendedDriversLoaded() {
            loading = false
        }
    }

    QQC2.BusyIndicator {
        running: kcmConn.loading
        visible: running
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        implicitWidth: Kirigami.Units.gridUnit * 6
        implicitHeight: Kirigami.Units.gridUnit * 6
    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        visible: !busy

        QQC2.Button {
            text: i18nc("@action:button", "Select Recommended Driver")
            icon.name: "dialog-ok-symbolic"
            visible: recmlist.count > 0

            onClicked: selectDriver(kcm.recommendedDrivers[recmlist.currentIndex])

            QQC2.ToolTip {
                text: i18nc("@info:tooltip", "Recommended drivers are based on printer make/model and connection type")
            }
        }

        QQC2.Button {
            text: i18nc("@action:button", "Manual Driver Search…")
            icon.name: "search-symbolic"

            onClicked: manualDriverSelect()

            QQC2.ToolTip {
                text: i18nc("@info:tooltip", "Search for and select a driver based on printer make/model")
            }
        }
    }

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        Component.onCompleted: {
            if (background) {
                background.visible = true
            }
        }

        contentItem: ListView {
            id: recmlist
            clip: true

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

                text: modelData.title
                subtitle: modelData["ppd-name"]
                icon.name: {
                    if (ippCapable) {
                        return modelData.favorite
                               ? "favorites-symbolic"
                               : "dialog-question-symbolic"
                    } else {
                        return modelData.match.startsWith("exact")
                               ? "favorites-symbolic"
                               : "dialog-question-symbolic"
                    }
                }

                highlighted: index === ListView.view.currentIndex
                onClicked: ListView.view.currentIndex = index
            }
        }
    }

}
