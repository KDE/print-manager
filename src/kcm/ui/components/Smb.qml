/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
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
    subtitle: i18nc("@title:group", "Find Printers on the Windows Network")
    helpText: i18nc("@info:usagetip", "Enter the address of the Windows Printer")
    showUriSearch: false
    showAddressExamples: true

    addressExamples: [
        "smb://[workgroup/]server[:port]/printer",
        "smb://[user:password]@[workgroup/]server[:port]/printer"
    ]

    scheme: compLoader.selector + "://"

    contentItem: Kirigami.FormLayout {
        Layout.alignment: Qt.AlignHCenter

        QQC2.TextField {
            id: username
            Kirigami.FormData.label: i18n("Username:")
        }

        Kirigami.PasswordField {
            id: password
            Kirigami.FormData.label: i18n("Password:")
        }

        QQC2.Button {
            text: i18nc("@action:button", "Save Printer…")
            icon.name: "dialog-ok-symbolic"
            Layout.alignment: Qt.AlignHCenter

            // TODO: fix url creation/validation, user/pass issues
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
                                 , "printer-info": "Windows Printer"})
                    manualDriverSelect()
                } else {
                    setError(i18n("Invalid Windows Printer URL: %1", uriStr))
                }

            }
        }
    }



}
 
