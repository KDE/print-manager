/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
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
    id: jobItem
    clip: true
    width: jobItem.ListView.view.width
    height: items.height + padding.margins.top + padding.margins.bottom

    Behavior on height { PropertyAnimation {} }

    property bool currentItem: ListView.isCurrentItem
    property bool highlight: highlightPrinter === jobPrinter

    Keys.onDeletePressed: cancelJob()
    Keys.onReturnPressed: cancelJob()

    onCurrentItemChanged: updateSelection();
    onHighlightChanged: updateSelection();

    function updateSelection() {
        var containsMouse = mouseArea.containsMouse;

        if (currentItem && containsMouse) {
            padding.opacity = 1;
        } else if (currentItem) {
            padding.opacity = 0.9;
        } else if (containsMouse) {
            padding.opacity = 0.7;
        } else {
            padding.opacity = 0;
        }
    }

    function cancelJob() {
        cancelButton.enabled = false;
        jobsModel.cancel(jobPrinter, jobId);
        cancelButton.enabled = true;
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
        hoverEnabled: true
        onEntered: updateSelection()
        onExited: updateSelection()
        onClicked: {
            if (currentItem) {
                jobItem.ListView.view.currentIndex = -1;
            } else {
                jobItem.ListView.view.currentIndex = index;
                jobItem.forceActiveFocus();
            }
            updateSelection();
        }
        onDoubleClicked: toggleChangelog()
    }
        
    Column {
        id: items
        spacing: 4
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: padding.margins.top
        anchors.leftMargin: padding.margins.left
        anchors.rightMargin: padding.margins.right
        anchors.bottomMargin: padding.margins.bottom
        Row {
            id: jobRow
            spacing: 4
            width: parent.width
            QIconItem {
                id: jobIcon
                width: parent.height
                height: width
                anchors.verticalCenter: parent.verticalCenter
                icon: QIcon(jobIconName)
            }
            PlasmaComponents.Label {
                // 12 = 3 * spacing
                id: jobNameLabel
                width: parent.width - pagesLabel.width - jobIcon.width - parent.spacing * 2
                anchors.verticalCenter: parent.verticalCenter
                height: paintedHeight
                elide: Text.ElideRight
                text: jobName
            }
            PlasmaComponents.Label {
                id: pagesLabel
                visible: jobPages != 0
                width: paintedWidth
                height: paintedHeight
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignRight
                text: jobPages
                font.pointSize: theme.smallestFont.pointSize
                color: "#99"+(theme.textColor.toString().substr(1))
            }
        }

        Row {
            id: actionRow
            opacity: currentItem ? 1 : 0
            width: parent.width

            spacing: 4
            Column {
                id: columnButton
                spacing: 2
                PlasmaComponents.ToolButton {
                    id: cancelButton
                    flat: true
                    iconSource: "dialog-cancel"
                    text:  i18n("Cancel Job")
                    visible: jobCancelEnabled
                    onClicked: cancelJob()
                }
                PlasmaComponents.ToolButton {
                    id: holdButton
                    flat: true
                    iconSource: "document-open-recent"
                    text: jobRestartEnabled ?  i18n("Reprint Job") : (jobHoldEnabled ?  i18n("Hold Job") :  i18n("Release Job"))
                    visible: jobCancelEnabled || jobRestartEnabled
                    onClicked: {
                        enabled = false;
                        if (jobHoldEnabled) {
                            jobsModel.hold(jobPrinter, jobId);
                        } else {
                            jobsModel.release(jobPrinter, jobId);
                        }
                        enabled = true;
                    }
                }
            }
            Column {
                id: column
                PlasmaComponents.Label {
                    id: ownerLabel
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) {
                        parent.width = paintedWidth;
                        }
                    }
                    text: i18n("Owner:")
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                PlasmaComponents.Label {
                    id: sizeLabel
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) {
                        parent.width = paintedWidth;
                        }
                    }
                    text: i18n("Size:")
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                PlasmaComponents.Label {
                    id: createdLabel
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    onPaintedWidthChanged: {
                        if (paintedWidth > parent.width) {
                        parent.width = paintedWidth;
                        }
                    }
                    text: i18n("Created:")
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
            }
            Column {
                width: parent.width - columnButton.width - column.width - parent.spacing * 2
                PlasmaComponents.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                    text: jobOwner
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                PlasmaComponents.Label {
                    height:paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                    text: jobSize
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
                PlasmaComponents.Label {
                    height: paintedHeight
                    width: parent.width
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                    text: jobCreatedAt
                    font.pointSize: theme.smallestFont.pointSize
                    color: "#99"+(theme.textColor.toString().substr(1))
                }
            }
        }
    }
}

