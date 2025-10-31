/*
 *   SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PM

pragma ComponentBehavior: Bound

Kirigami.Page {
    id: root

    property int deviceCount: 0
    property bool serverAvailable: false

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Component {
        id: noDevicesMsgComp

        Kirigami.PlaceholderMessage {
            icon.name: "printer-warning"
            text: i18nc("@info:status", "No printers are currently configured")
            helpfulAction: Kirigami.Action {
                text: i18nc("@action:button", "Configure…")
                icon.name: "printer-symbolic"
                onTriggered: PM.ProcessRunner.openKCM()
            }
        }
    }

    Component {
        id: noServiceMsgComp

        Kirigami.PlaceholderMessage {
            icon.name: "printer-error"
            text: i18nc("@info:status", "Printing services not available")
            explanation: xi18nc("@info:usagetip", "Verify that the CUPS system service is active.")
        }
    }

    Component {
        id: genericComp

        Kirigami.PlaceholderMessage {
            icon.name: "printer-error"
            text: i18nc("@info:status", "Unknown error condition with Printing Services")
        }
    }

    Loader {
        id: compLoader
        anchors.centerIn: parent
        width: root.width - (Kirigami.Units.gridUnit * 4)
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

        active: true

        sourceComponent: {
            let ret = null
            if (!serverAvailable) {
                ret = noServiceMsgComp
            } else if (deviceCount === 0) {
                ret = noDevicesMsgComp
            } else {
                ret = genericComp
            }

            return ret
        }
    }

}
