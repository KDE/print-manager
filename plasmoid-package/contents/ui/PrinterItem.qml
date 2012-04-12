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
    state: stateEnum

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
            highlightPrinter = "";
        }
        onClicked: {
            if (printerItem.ListView.view.currentIndex == index) {
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
    
    Item {
        anchors {
            fill: parent
            topMargin: padding.margins.top
            leftMargin: padding.margins.left
            rightMargin: padding.margins.right
            bottomMargin: padding.margins.bottom
        }
        QIconItem {
            id: printerIcon
            width: 32
            height: 32
            icon: QIcon(iconName)
            anchors {
                left: parent.left
                top: parent.top
            }
            Behavior on opacity { PropertyAnimation {} }
        }
        
        Column {
            id: labelsColumn
            spacing: padding.margins.top/2
            anchors {
                top: parent.top
                left: printerIcon.right
                right: switchAction.left
                leftMargin: padding.margins.left
            }
            PlasmaComponents.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                height: paintedHeight
                elide: Text.ElideRight
                text: info
            }
            
            PlasmaComponents.Label {
                anchors.left: parent.left
                anchors.right: parent.right
                height: paintedHeight
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
            anchors {
                right: parent.right
                verticalCenter: printerIcon.verticalCenter
            }
            checked: stateEnum == "stopped" ? false : true
            onClicked: {
                service = printersModel.dataSource.serviceForSource(DataEngineSource);
                operation = service.operationDescription(checked ? "resume" : "pause");
                service.startOperationCall(operation);
            }
        }
    }
    
    states: [
        State {
            name: "stopped"
            PropertyChanges { target: labelsColumn; opacity: 0.6}
            PropertyChanges { target: printerIcon; opacity: 0.6}
        },
        State {
            PropertyChanges { target: labelsColumn; opacity: 1}
            PropertyChanges { target: printerIcon; opacity: 1}
        }
    ]
}

