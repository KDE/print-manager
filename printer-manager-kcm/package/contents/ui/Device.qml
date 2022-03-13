/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as QQC2

import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.2

import org.kde.bluezqt 1.0 as BluezQt

SimpleKCM {

    id: root

    property var modelData

    title: modelData.printerName

    header: Kirigami.InlineMessage {
        id: error
        type: Kirigami.MessageType.Error
        showCloseButton: true

        Connections {
            target: kcm

            function onRequestError(errorMessage) {
                error.text = errorMessage
                error.visible = true
            }
        }
    }

    ColumnLayout {

        Kirigami.Icon {
            source: modelData.iconName
            Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter
        }

        Kirigami.FormLayout {

            QQC2.CheckBox {
                text: i18n("Default printer")
                enabled: !checked
                checked: root.modelData.isDefault
                onToggled: kcm.makePrinterDefault(root.modelData.printerName)
            }

            QQC2.CheckBox {
                text: root.modelData.isClass ? i18n("Share this class") : i18n("Share this printer")
                checked: enabled && root.modelData.isShared
                enabled: kcm.shareConnectedPrinters
                onToggled: kcm.makePrinterShared(root.modelData.printerName, checked,  root.modelData.isClass)
            }

            QQC2.CheckBox {
                text: i18n("Reject print jobs")
                checked: !root.modelData.isAcceptingJobs
                onToggled: {
                    kcm.makePrinterRejectJobs(root.modelData.printerName, checked)
                }
            }

            QQC2.Label {
                text: root.modelData.kind
                Kirigami.FormData.label: i18n("Kind:")
            }

            QQC2.Label {
                text: root.modelData.location
                visible: text !== ""
                Kirigami.FormData.label: i18n("Location:")
            }

            QQC2.Button {
                text: i18n("Print Test Page")
                onClicked: kcm.printTestPage(root.modelData.printerName, root.modelData.isClass)
            }

            QQC2.Button {
                text: i18n("Print Self Test Page")
                visible: root.modelData.commands.indexOf("PrintSelfTestPage") !== -1
                onClicked: kcm.printSelfTestPage(root.modelData.printerName)
            }

            QQC2.Button {
                text: i18n("Clean Print Heads")
                visible: root.modelData.commands.indexOf("Clean") !== -1
                onClicked: kcm.cleanPrintHeads(root.modelData.printerName)
            }

            QQC2.Button {
                text: i18n("Configure")
                onClicked: kcm.configurePrinter(root.modelData.printerName)
            }

            QQC2.Button {
                text: i18n("Open Print Queue")
                onClicked: kcm.openPrintQueue(root.modelData.printerName)
            }

            Repeater {
                model: root.modelData.markers["marker-names"]

                delegate: QQC2.ProgressBar {
                    from: 0
                    to: 100
                    value: root.modelData.markers["marker-levels"][index]
                    palette.highlight: root.modelData.markers["marker-colors"][index]
                    Kirigami.FormData.label: modelData
                }
            }
        }
    }
}
