/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.plasma.printmanager as PM

pragma ComponentBehavior: Bound

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

    QQC2.Label {
        text: i18nc("@info:label", "Recommended Drivers:" )
    }

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        Kirigami.StyleHints.showFramedBackground: true

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

            delegate: QQC2.ItemDelegate {
                id: del
                width: ListView.view.width

                required property var modelData
                required property int index

                text: modelData.title

                contentItem: KD.IconTitleSubtitleWithActions {
                    title: del.text
                    subtitle: del.modelData["ppd-name"]
                    icon.name: {
                        if (root.ippCapable) {
                            return del.modelData.favorite
                                   ? "favorites-symbolic"
                                   : "dialog-question-symbolic"
                        } else {
                            return del.modelData.match.startsWith("exact")
                                   ? "favorites-symbolic"
                                   : "dialog-question-symbolic"
                        }
                    }

                    actions: Kirigami.Action {
                        text: i18nc("@action:button", "Select Driver…")
                        icon.name: "dialog-ok-symbolic"
                        tooltip: i18nc("@info:tooltip", "Select this driver for the printing device")

                        onTriggered: root.selectDriver(kcm.recommendedDrivers[del.index])
                    }
                }
            }
        }
    }

    QQC2.Button {
        text: i18nc("@action:button", "Manual Driver Search…")
        icon.name: "search-symbolic"
        visible: !root.busy
        Layout.alignment: Qt.AlignHCenter

        onClicked: manualDriverSelect()

        QQC2.ToolTip {
            text: i18nc("@info:tooltip", "Search for and select a driver based on printer make/model")
        }
    }

}
