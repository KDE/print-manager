/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 
import QtQuick.Layouts 
import QtQuick.Controls as QQC2
import org.kde.kirigami 2 as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: root

    readonly property bool isPaused: kcm.printerModel.printerState === 5

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    extraFooterTopPadding: false

    actions: [
        Kirigami.Action {
            text: i18n("Add…")
            icon.name: "list-add-symbolic"
            onTriggered: kcm.addPrinter()
        }
        , Kirigami.Action {
            text: i18n("Configure Global Settings")
            icon.name: "configure-symbolic"
            onTriggered: kcm.push("Global.qml")
        }
    ]

    view: ListView {
        id: list
        clip: true

        model: kcm.printerModel

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: list.count === 0
            icon.name: "printer-symbolic"
            text: i18n("No printers are currently set up")

            helpfulAction: Kirigami.Action {
                icon.name: "list-add-symbolic"
                text: i18n("Add Printer…")
                onTriggered: kcm.addPrinter()
            }
        }

        delegate: Kirigami.BasicListItem {
            text: model.printerName + (kcm.printerModel.sourceModel.displayLocationHint ? " (%1)".arg(model.location) : "")
            subtitle: model.stateMessage
            icon.name: model.iconName
           
            font.bold: list.count > 1 && model.isDefault
            hoverEnabled: false
            highlighted: false

            // TODO: Attempting to "nullify" a pressed or clicked event,
            // apparently both of these have to be present.  The "new" way
            // should just need the background: null
            background: null
            down: false

            trailing: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.ToolButton {
                    text: i18n("Quick Settings")
                    icon.name: "configure-symbolic"
                    display: QQC2.AbstractButton.IconOnly

                    QQC2.ToolTip {
                        text: parent.text
                    }

                    onClicked: kcm.push("PrinterSettings.qml", { modelData: model })
                }

                QQC2.ToolButton {
                    text: i18n("Open Print Queue…")
                    icon.name: "view-list-details-symbolic"
                    display: QQC2.AbstractButton.IconOnly

                    QQC2.ToolTip {
                        text: parent.text
                    }

                    onClicked: kcm.openPrintQueue(model.printerName)
                }

                QQC2.ToolButton {
                    icon.name: isPaused ? "media-playback-start-symbolic" : "media-playback-pause-symbolic"
                    text: isPaused ? i18n("Resume") : i18n("Pause")

                    onClicked: {
                        if (isPaused) {
                            kcm.resumePrinter(model.printerName);
                        } else {
                            kcm.pausePrinter(model.printerName);
                        }
                    }

                    QQC2.ToolTip {
                        text: isPaused ? i18n("Resume printing") : i18n("Pause printing")
                    }
                }

            }

        }
    }
}
