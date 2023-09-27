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
    spacing: Kirigami.Units.largeSpacing*2

    Layout.fillWidth: true
    Layout.fillHeight: true

    property alias title: heading.title
    property alias subtitle: heading.subtitle
    property alias icon: heading.icon
    property alias helpText: helpLabel.text

    property list<Kirigami.Action> actions: []

    Component.onDestruction: {
        kcm.clearRecommendedDrivers()
        kcm.clearRemotePrinters()
    }

    Kirigami.IconTitleSubtitle {
        id: heading

        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.15
        subtitleFont.pointSize: Kirigami.Theme.defaultFont.pointSize

        elide: Text.ElideRight

        icon.source: "printer"
        icon.width: Kirigami.Units.iconSizes.huge
        icon.height: Kirigami.Units.iconSizes.huge

        Layout.alignment: Qt.AlignHCenter
        Layout.bottomMargin: Kirigami.Units.largeSpacing
    }

    Kirigami.Heading {
        id: helpLabel
        level: 4
        visible: text.length > 0
        horizontalAlignment: Qt.AlignHCenter
        Layout.fillWidth: true
    }

    RowLayout {
        visible: actions.length > 0
        Layout.alignment: Qt.AlignHCenter
        Repeater {
            model: actions
            QQC2.Button {
                action: modelData
                visible: modelData.visible
                enabled: modelData.enabled
            }
        }
    }
}
