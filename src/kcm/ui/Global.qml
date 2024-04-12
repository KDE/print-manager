/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 
import QtQuick.Layouts 
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

KCM.AbstractKCM {
    title: i18nc("@title:window", "Print Server Settings")

    headerPaddingEnabled: false

    header: BannerWithTimer {
        id: error

        onTimeout: kcm.pop()
    }

    footer: RowLayout {
        Layout.margins: Kirigami.Units.largeSpacing

        Kirigami.UrlButton {
            text: i18nc("@action:button", "CUPS Print Server Admin")
            url: "http://localhost:631/admin"
            padding: Kirigami.Units.largeSpacing
        }

        Item { Layout.fillWidth: true }

        QQC2.Button {
            text: i18nc("@action:button Reset to current settings", "Reset")
            icon.name: "edit-reset-symbolic"
            enabled: settings.hasPending
            onClicked: settings.reset()
        }

        QQC2.Button {
            text: i18nc("@action:button Apply new settings", "Apply")
            icon.name: "dialog-ok-apply"
            enabled: settings.hasPending
            onClicked: {
                kcm.saveServerSettings(settings.pending)
                settings.hasPending = false
            }
        }
    }

    Component.onCompleted: {
        if (!kcm.serverSettingsLoaded)
            kcm.getServerSettings()
        else
            conn.onServerSettingsChanged()
    }

    // Each item stores its corresponding CUPS field name in objectName
    // This is then used to generate the "pending" changes map
    ConfigValues {
        id: settings
        usePendingCount: false
    }

    Connections {
        id: conn
        target: kcm

        function onRequestError(errorMessage) {
            error.text = errorMessage
            error.visible = true
            layout.enabled = false
        }

        function onServerSettingsChanged() {
            settings.init(kcm.serverSettings)
            layout.enabled = true
        }
    }

    component SettingCheckBox: QQC2.CheckBox {
        checked: settings.value(objectName) ?? false
        onToggled: {
            settings.add(objectName, checked)
        }
    }

    ColumnLayout {
        id: layout
        enabled: false
        anchors.centerIn: parent

        Kirigami.Icon {
            source: "printer"
            Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
        }

        SettingCheckBox {
            id: share
            objectName: "_share_printers"
            text: i18nc("@option:check", "Share printers connected to this system")
            onToggled: {
                if (!checked) {
                    netPrint.checked = false
                    netPrint.toggled()
                }
            }
        }

        SettingCheckBox {
            id: netPrint
            objectName: "_remote_any"
            text: i18nc("@option:check", "Allow printing from the Internet")
            enabled: share.checked
        }

        SettingCheckBox {
            objectName: "_remote_admin"
            text: i18nc("@option:check", "Allow remote administration")
        }

        SettingCheckBox {
            objectName: "_user_cancel_any"
            text: i18nc("@option:check", "Allow users to cancel any job (not just their own)")
        }


    }
}
