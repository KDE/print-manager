/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * Device setup for Serial printer devices (Legacy)
*/
BaseDevice {
    title: compLoader.info
    subtitle: i18nc("@title:group", "Serial Printer")
    helpText: i18nc("@info:usagetip", "Enter the Serial Printer settings")
    showUriSearch: false

    property int maxBaud: 19200
    property var baudValues: [
        1200,
        2400,
        4800,
        9600,
        19200,
        38400,
        57600,
        115200,
        230400,
        460800
    ]

    property var baudModel: []

    contentItem: Kirigami.FormLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        // TODO: set values from found printer
        Component.onCompleted:  {
            baudValues.forEach(v => {
                                   if (v <= maxBaud) {
                                       baudModel.push(v)
                                   }
                               })
            baud.model = baudModel
            uriText = "http://localhost"
        }

        QQC2.ComboBox {
            id: baud
            Kirigami.FormData.label: i18n("Baud Rate:")
        }

        QQC2.ComboBox {
            id: parity
            Kirigami.FormData.label: i18n("Parity:")
            textRole: "text"
            valueRole: "value"
            model: [
                { value: "none", text: i18n("None") },
                { value: "even", text: i18n("Even") },
                { value: "odd", text: i18n("Odd") }
            ]

        }

        QQC2.ComboBox {
            id: bits
            Kirigami.FormData.label: i18n("Data Bits:")
            model: ["8", "7"]
        }

        QQC2.ComboBox {
            id: flow
            Kirigami.FormData.label: i18n("Flow Control:")
            textRole: "text"
            valueRole: "value"
            model: [
                { value: "none", text: i18n("None") },
                { value: "soft", text: i18n("XON/XOFF (Software)") },
                { value: "hard", text: i18n("RTS/CTS (Hardware)") },
                { value: "dtrdsr", text: i18n("DTR/DSR (Hardware)") }
            ]
        }

        QQC2.Button {
            text: i18nc("@action:button", "Save Printer…")
            icon.name: "dialog-ok-symbolic"
            Layout.alignment: Qt.AlignHCenter

            onClicked: {
                // TODO: fix URI defn
                const uri = uriText // settings.value("device-uri")
                // const pos = uri.indexOf('?')
                const args = "?baud=%1+bits=%2+parity=%3+flow=%4"
                        .arg(baud.currentValue)
                        .arg(bits.currentValue)
                        .arg(parity.currentValue)
                        .arg(flow.currentValue)
                print("SERIAL", uri, args)

                // const ret = uri.replace(pos, uri.length - pos, args)
                const ret = uri + args
                settings.set({"device-uri": ret
                             , "printer-info": "Serial Printer"})

                // uriText = ret
                manualDriverSelect()
            }
        }

    }
}
