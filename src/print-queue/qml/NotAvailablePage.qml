/*
 *   SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PM
import org.kde.coreaddons as KAddons

pragma ComponentBehavior: Bound

Kirigami.Page {
    id: root

    property int deviceCount: 0
    property bool serverAvailable: false

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Component {
        id: noDevicesMsgComp

        Kirigami.PlaceholderMessage {
            icon.name: "printer-warning"
            text: i18nc("@info:status", "No printers have been set up")
            helpfulAction: Kirigami.Action {
                text: i18nc("@action:button", "Set Up a Printer…")
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
            explanation: i18nc("@info:usagetip", "Verify that the CUPS system service is active or check: <a href=\"%1\">%2 Support</a>", KAddons.KOSRelease.supportUrl, KAddons.KOSRelease.prettyName)
            onLinkActivated: link => Qt.openUrlExternally(link)
        }
    }

    Component {
        id: genericComp

        Kirigami.PlaceholderMessage {
            icon.name: "printer-error"
            text: i18nc("@info:status", "Unknown error condition with printing services")
            explanation: i18nc("@info:usagetip", "Check for issues here: <a href=\"%1\">%2 Issues</a>", KAddons.KOSRelease.bugReportUrl, KAddons.KOSRelease.prettyName)
            onLinkActivated: link => Qt.openUrlExternally(link)
        }
    }

    Loader {
        id: compLoader
        anchors.centerIn: parent
        width: root.width - (Kirigami.Units.gridUnit * 4)
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

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
