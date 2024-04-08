/**
 SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * Device setup for SMB (Windows) printer devices,
*/
BaseDevice {
    title: compLoader.info
    subtitle: i18nc("@title:group", "Set up a printer on a Windows Network")
    helpText: i18nc("@info:usagetip", "Enter the address of the Windows Printer")
    showUriSearch: false
    showAddressExamples: true

    addressExamples: [
        "smb://[workgroup/]server[:port]/printer_name",
        "smb://[user:password]@[workgroup/]server[:port]/printer_name",
        "smb://username:password@domain/windows_print_server_host_name/printer_name"
    ]

    scheme: compLoader.selector + "://"

    contentItem: Kirigami.FormLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignHCenter

        Component.onCompleted: {
            const url = getUrl(settings.value("device-uri"))
            if (url) {
                uriText = url.href
            } else {
                uriText = scheme
            }
        }

        QQC2.TextField {
            id: username
            Kirigami.FormData.label: i18nc("@label:textbox Authentication user name", "Username:")
        }

        Kirigami.PasswordField {
            id: password
            Kirigami.FormData.label: i18nc("@label:textbox Authentication password", "Password:")
        }

        QQC2.Button {
            text: i18nc("@action:button Accept current values and continue", "Save Printer…")
            icon.name: "dialog-ok-symbolic"
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                let uriStr = uriText
                if (!uriStr.startsWith(scheme)) {
                    uriStr = scheme + uriStr
                }
                // validate url
                const url = getUrl(uriStr)
                if (url) {
                    if (username.text) {
                        url.username = username.text
                        if (password.text) {
                            url.password = password.text
                        }
                    }
                    settings.set({"device-uri": url.href
                                 , "printer-info": "Printer@" + url.host})
                    manualDriverSelect()
                } else {
                    setError(i18nc("@info:status", "Invalid Printer URL: %1", uriStr))
                }

            }
        }
    }
}
 
