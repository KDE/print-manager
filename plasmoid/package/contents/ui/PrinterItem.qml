/*
 *   Copyright 2012 Daniel Nicoletti <dantti12@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

Item {
    id: printerItem
    width: ListView.view.width
    height: items.height + padding.margins.top + padding.margins.bottom
    state: isPaused ? "PAUSED" : "*"

    property bool currentItem: ListView.isCurrentItem
    property bool multipleItems: false

    Keys.onSpacePressed: toggleSelection()

    onCurrentItemChanged: updateSelection();

    function updateSelection() {
        var containsMouse = mouseArea.containsMouse;

        if (currentItem && containsMouse) {
            highlightPrinter = printerName;
            padding.opacity = 1;
        } else if (currentItem) {
            padding.opacity = 0.9;
        } else if (containsMouse) {
            padding.opacity = 0.7;
        } else {
            if (highlightPrinter === printerName) {
                highlightPrinter = "";
            }
            padding.opacity = 0;
        }
    }

    function toggleSelection() {
        switchAction.enabled = false;
        if (isPaused) {
            if (!isAcceptingJobs) {
                printersModel.acceptJobs(printerName);
            }
            if (printerState === 5) {
                printersModel.resumePrinter(printerName);
            }
        } else {
            printersModel.pausePrinter(printerName);
        }
        switchAction.enabled = true;
    }

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
        Behavior on opacity { PropertyAnimation {} }
        anchors.fill: parent
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: multipleItems
        onEntered: updateSelection()
        onExited: updateSelection()
        onClicked: {
            if (currentItem) {
                printerItem.ListView.view.currentIndex = -1;
                filterJobs = "";
//                highlightPrinter = "";
            } else if (multipleItems) {
                printerItem.ListView.view.currentIndex = index;
                printerItem.forceActiveFocus();
                // We need to unset the filter before applying a new one
                // otherwise, the filter model get's rowCoun() == 0 and
                // the popup hides
                filterJobs = "";
                filterJobs = printerName;
//                highlightPrinter = "";
            }
            updateSelection();
        }
    }

    Row {
        id: items
        width: parent.width - padding.margins.left - padding.margins.right
        anchors.topMargin: padding.margins.top
        anchors.leftMargin: padding.margins.left
        anchors.rightMargin: padding.margins.right
        anchors.bottomMargin: padding.margins.bottom
        anchors.centerIn: parent
        spacing: 4

        QIconItem {
            id: printerIcon
            width: parent.height
            height: width
            icon: QIcon(iconName)
            Behavior on opacity { PropertyAnimation {} }
        }

        Column {
            id: labelsColumn
            width: parent.width - printerIcon.width - switchAction.width - parent.spacing * 2
            spacing: padding.margins.top/2
            Row {
                id: nameRow
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 4
                PlasmaComponents.Label {
                    id: printerLabel
                    height: paintedHeight
                    elide: Text.ElideRight
                    text: printerName
                }
                PlasmaComponents.Label {
                    id: printerDescription
                    width: parent.width - printerLabel.paintedWidth
                    height: printerLabel.height
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignBottom
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                    text: info
                }
            }
            PlasmaComponents.Label {
                height: paintedHeight
                anchors.left: parent.left
                anchors.right: parent.right
                verticalAlignment: Text.AlignTop
                text: stateMessage
                elide: Text.ElideRight
                font.italic: true
                font.pointSize: theme.smallestFont.pointSize
                color: "#99"+(theme.textColor.toString().substr(1))
            }
        }

        PlasmaComponents.Switch {
            id: switchAction
            anchors.verticalCenter: parent.verticalCenter
            focus: false
            checked: !isPaused
            onClicked: toggleSelection()
        }
    }

    states: [
        State {
            name: "PAUSED"
            PropertyChanges { target: labelsColumn; opacity: 0.6}
            PropertyChanges { target: printerIcon; opacity: 0.6}
        }
    ]
}
