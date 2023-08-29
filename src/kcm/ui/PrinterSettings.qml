/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 
import QtQuick.Layouts 
import QtQuick.Controls as QQC2
import org.kde.kirigami 2 as Kirigami
import org.kde.kirigamiaddons.labs.components as KAddons
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property var modelData

    title: modelData.info + (kcm.printerModel.sourceModel.displayLocationHint ? " (%1)".arg(modelData.location) : "")

    actions: [
        Kirigami.Action {
            text: i18n("Configure…")
            icon.name: "configure-symbolic"
            onTriggered: kcm.configurePrinter(modelData.printerName)
        }
        , Kirigami.Action {
            text: i18n("Remove Printer")
            icon.name: "edit-delete-remove-symbolic"
            onTriggered: removePrompt.open()
        }
    ]

    Connections {
        target: kcm

        function onRequestError(errorMessage) {
            error.text = errorMessage
            error.visible = true
        }

        function onRemoveComplete(removed) {
            if (removed) {
                kcm.currentIndex = 0
            } else {
                error.text = i18n("Failed to remove the printer: ") + error.text
            }
        }
    }

    header: KAddons.Banner {
        id: error
        type: Kirigami.MessageType.Error
        title: i18n("Printer Config Error")
        showCloseButton: true
    }

    Kirigami.PromptDialog {
        id: removePrompt
        title: i18n("Remove Printer")
        subtitle: "%1 %2?".arg(i18n("Are you sure you really want to remove")).arg(modelData.printerName)

        standardButtons: Kirigami.Dialog.NoButton

        customFooterActions: [
            Kirigami.Action {
                text: i18n("Remove Printer")
                icon.name: "edit-delete-remove-symbolic"
                onTriggered: {
                    kcm.removePrinter(modelData.printerName)
                    removePrompt.close()
                }
            },
            Kirigami.Action {
                text: i18n("Cancel")
                icon.name: "dialog-cancel-symbolic"
                onTriggered: removePrompt.close()
            }
        ]
    }

    ColumnLayout {
        anchors.centerIn: parent

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: modelData.iconName
                Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
                Layout.preferredHeight: Layout.preferredWidth
                Layout.alignment: Qt.AlignHCenter
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    text: modelData.info
                    level: 3
                    type: Kirigami.Heading.Type.Primary
                }

                Kirigami.Heading {
                    text: modelData.kind
                    level: 5
                    type: Kirigami.Heading.Type.Secondary
                }

                QQC2.CheckBox {
                    text: i18n("Default printer")
                    checked: modelData.isDefault
                    onToggled: kcm.makePrinterDefault(modelData.printerName)
                }

                QQC2.CheckBox {
                    text: modelData.isClass ? i18n("Share this class") : i18n("Share this printer")
                    checked: enabled && modelData.isShared
                    enabled: kcm.shareConnectedPrinters
                    onToggled: kcm.makePrinterShared(modelData.printerName, checked, modelData.isClass)
                }

                QQC2.CheckBox {
                    text: i18n("Reject print jobs")
                    checked: !modelData.isAcceptingJobs
                    onToggled: kcm.makePrinterRejectJobs(modelData.printerName, checked)
                }

            }
        }

        Item { Layout.topMargin: Kirigami.Units.largeSpacing }

        Repeater {
            model: modelData.markers["marker-names"]

            delegate: RowLayout {
                QQC2.Label {
                    text: modelData
                    Layout.minimumWidth: Kirigami.Units.gridUnit*7
                }

                QQC2.ProgressBar {
                    from: 0
                    to: 100
                    value: root.modelData.markers["marker-levels"][index]
                    palette.highlight: root.modelData.markers["marker-colors"][index]
                }
            }
        }

        Item { Layout.topMargin: Kirigami.Units.largeSpacing }

        RowLayout {

            QQC2.Button {
                text: i18n("Print Test Page")
                icon.name: "document-print-symbolic"
                onClicked: kcm.printTestPage(modelData.printerName, modelData.isClass)
            }

            QQC2.Button {
                text: i18n("Print Self-Test Page")
                icon.name: "document-print-symbolic"
                visible: modelData.commands.indexOf("PrintSelfTestPage") !== -1
                onClicked: kcm.printSelfTestPage(modelData.printerName)
            }

            QQC2.Button {
                text: i18n("Clean Print Heads")
                icon.name: "document-cleanup-symbolic"
                visible: modelData.commands.indexOf("Clean") !== -1
                onClicked: kcm.cleanPrintHeads(modelData.printerName)
            }
        }
    }
}
