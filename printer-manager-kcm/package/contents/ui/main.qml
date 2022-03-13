/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10 as QQC2

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kcm 1.2

import org.kde.bluezqt 1.0 as BluezQt

import org.kde.plasma.private.bluetooth 1.0

ScrollViewKCM {

    id: root

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    view: ListView {
        id: list
        clip: true

        model: kcm.printerModel

        delegate: Kirigami.BasicListItem {
            text: model.printerName
            subtitle: model.stateMessage
            icon: model.iconName
            iconSize: Kirigami.Units.iconSizes.medium

            trailing: QQC2.ToolButton {
                text: i18n("Remove")
                icon.name: "list-remove"
                display: QQC2.AbstractButton.IconOnly

                onClicked: {
                    // TODO confirmation dialog?
                    kcm.removePrinter(model.printerName)
                }
            }

            onClicked: kcm.push("Device.qml", {modelData: model})
        }
    }

    footer: RowLayout {

        QQC2.Button {
            text: i18n("Add...")
            icon.name: "list-add"
            onClicked: kcm.addPrinter()
        }

        Item {
            Layout.fillWidth: true
        }

        QQC2.Button {
            text: i18n("Configure...")
            icon.name: "configure"
            onClicked: kcm.push("General.qml")
        }
    }
}
