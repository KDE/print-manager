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

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

Item {
    id: printerItem
    property bool multipleItems
    width: printerItem.ListView.view.width
    height: 50
    state: isPaused ? "PAUSED" : ""

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
        anchors.fill: parent
    }
    MouseArea {
        id: container
        anchors.fill: parent
        hoverEnabled: multipleItems
        onEntered: {
            padding.opacity = 0.6;
            highlightPrinter = printerName;
        }
        onExited: {
            padding.opacity = 0;
            if (highlightPrinter === printerName) {
                highlightPrinter = "";
            }
        }
        onClicked: {
            if (printerItem.ListView.view.isCurrentIndex) {
                printerItem.ListView.view.currentIndex = -1;
                whichPrinter = printerName;
                highlightPrinter = "";
            } else if (multipleItems) {
                printerItem.ListView.view.currentIndex = index;
                whichPrinter = "";
                highlightPrinter = "";
            }
        }
    }

    Row {
        anchors.fill: parent
        anchors.topMargin: padding.margins.top
        anchors.leftMargin: padding.margins.left
        anchors.rightMargin: padding.margins.right
        anchors.bottomMargin: padding.margins.bottom
        spacing: 4
        QIconItem {
            id: printerIcon
            width: parent.height
            height: width
            anchors.verticalCenter: parent.verticalCenter
            icon: QIcon(iconName)
            Behavior on opacity { PropertyAnimation {} }
        }
        
        Column {
            id: labelsColumn
            width: parent.width - printerIcon.width - switchAction.width - parent.spacing * 2
            anchors.verticalCenter: parent.verticalCenter
            spacing: padding.margins.top/2
            PlasmaComponents.Label {
                id: printerLabel
                anchors.left: parent.left
                anchors.right: parent.right
                height: paintedHeight
                elide: Text.ElideRight
                text: info
            }
            PlasmaComponents.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                height: printerLabel.height
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
            opacity: 1
            anchors.verticalCenter: parent.verticalCenter
            checked: !isPaused
            onClicked: {
                enabled = false;
                if (checked) {
                    if (!isAcceptingJobs) {
                        printersModel.acceptJobs(printerName);
                    }
                    if (printerState === 5) {
                        printersModel.resumePrinter(printerName);
                    }
                } else {
                    printersModel.pausePrinter(printerName);
                }
                enabled = true;
            }
        }
    }
    
    states: [
        State {
            name: "PAUSED"
            PropertyChanges { target: labelsColumn; opacity: 0.6}
            PropertyChanges { target: printerIcon; opacity: 0.6}
        },
        State {
            PropertyChanges { target: labelsColumn; opacity: 1}
            PropertyChanges { target: printerIcon; opacity: 1}
        }
    ]
}
