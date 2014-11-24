/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   Copyright 2014 Jan Grulich <jgrulich@redhat.com>
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

import QtQuick 2.2
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

PlasmaComponents.ListItem {
    id: jobItem

    enabled: true
    width: jobItem.ListView.view.width
    height: items.height + (Math.round(units.gridUnit / 3) * 2)

    Keys.onDeletePressed: cancelJob()
    Keys.onReturnPressed: cancelJob()

    Column {
        id: items

        spacing: 4

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        Row {
            id: jobRow
            spacing: 4
            width: parent.width

            KQuickControlsAddons.QIconItem {
                id: jobIcon
                width: parent.height
                height: width
                anchors.verticalCenter: parent.verticalCenter
                icon: jobIconName
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

        Column {
            id: actionRow
            width: parent.width
            spacing: 4

            Row {
                id: columnButton
                width: parent.width
                spacing: 2

                PlasmaComponents.Button {
                    id: cancelButton
                    focus: true
                    width: holdButton.visible ? parent.width * 0.5 - parent.spacing : minimumWidth
                    KeyNavigation.tab: holdButton
                    KeyNavigation.backtab: holdButton
                    iconSource: "dialog-cancel"
                    text:  i18n("Cancel")
                    visible: jobCancelEnabled
                    onClicked: cancelJob()
                }

                PlasmaComponents.Button {
                    id: holdButton
                    focus: true
                    width: holdButton.visible ? parent.width * 0.5 - parent.spacing : minimumWidth
                    KeyNavigation.tab: cancelButton
                    KeyNavigation.backtab: cancelButton
                    iconSource: "document-open-recent"
                    text: jobRestartEnabled ?  i18n("Reprint") : (jobHoldEnabled ?  i18n("Hold") :  i18n("Release"))
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

            Row {
                id: detailsRow
                width: parent.width
                spacing: 4

                Column {
                    id: labelsColumn

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
                    width: parent.width - labelsColumn.width - parent.spacing * 2
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

    function cancelJob() {
        cancelButton.enabled = false;
        jobsModel.cancel(jobPrinter, jobId);
        cancelButton.enabled = true;
    }
}

