/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   Copyright 2014-2015 Jan Grulich <jgrulich@redhat.com>
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
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.printmanager 0.2 as PrintManager

Item {
    id: printerItem

    property bool isPaused: false
    property bool expanded: ListView.view.currentExpanded == index

    height: container.childrenRect.height + Math.round(units.gridUnit / 2)
    width: parent.width - Math.round(units.gridUnit / 2)

    MouseArea {
        id: container
        anchors {
            fill: parent
            topMargin: Math.round(units.gridUnit / 2)
            leftMargin: Math.round(units.gridUnit / 2)
            rightMargin: Math.round(units.gridUnit / 2)
            bottomMargin: Math.round(units.gridUnit / 2)
        }

        hoverEnabled: true
        onEntered: {
            printerItem.ListView.view.currentIndex = index;

            //this is done to hide the highlight if the mouse moves out of the list view
            //and we are not mouseoverring anything
            if (printerItem.ListView.view.highlightItem) {
                printerItem.ListView.view.highlightItem.opacity = 1
            }
        }
        onExited: {
            if (printerItem.ListView.view.highlightItem) {
                printerItem.ListView.view.highlightItem.opacity = 0
            }
        }
        onClicked: {
            printerItem.ListView.view.currentExpanded = expanded ? -1 : index;
            jobsFilterModel.filteredPrinters = expanded ? "" : printerName
        }

        KQuickControlsAddons.QIconItem {
            id: printerIcon

            anchors {
                left: parent.left
                verticalCenter: labelsColumn.verticalCenter
            }

            height: units.iconSizes.medium
            width: height
            icon: iconName
            Behavior on opacity { PropertyAnimation {} }
        }

        Column {
            id: labelsColumn

            anchors {
                top: parent.top
                left: printerIcon.right
                right: stateChangeButton.left
                leftMargin: Math.round(units.gridUnit / 2)
                rightMargin: Math.round(units.gridUnit / 2)
            }

            PlasmaComponents.Label {
                id: printerNameLabel

                anchors {
                    left: parent.left
                    right: parent.right
                }

                height: paintedHeight
                elide: Text.ElideRight
                font.weight: isDefault ? Font.DemiBold : Font.Normal
                text: printerName
            }

            PlasmaComponents.Label {
                id: printerStatusLabel

                anchors {
                    left: parent.left
                    right: parent.right
                }

                height: paintedHeight
                elide: Text.ElideRight
                font.pointSize: theme.smallestFont.pointSize
                opacity: 0.6
                text: stateMessage
            }
        }

        PlasmaComponents.ToolButton {
            id: stateChangeButton

            anchors {
                right: parent.right
                rightMargin: Math.round(units.gridUnit / 2)
                verticalCenter: printerIcon.verticalCenter
            }

            iconSource: isPaused ? "media-playback-start" : "media-playback-pause"
            opacity: container.containsMouse ? 1 : 0
            visible: opacity != 0

            onClicked: toggleSelection()
        }

        ListView {
            id: actionsList

            anchors {
                top: labelsColumn.bottom
                left: printerIcon.right
                right: stateChangeButton.left
                leftMargin: Math.round(units.gridUnit / 2)
                rightMargin: Math.round(units.gridUnit / 2)
            }
            interactive: false
            model: ListModel {
                Component.onCompleted: {
                    append({"name":i18n("Configure printer"), "icon":"configure", "actionType":1})
                    append({"name":i18n("Open print queue"), "icon":"view-list-details", "actionType":2})
                }
            }
            property int actionIconHeight: units.iconSizes.medium * 0.8
            height: expanded ? ((actionIconHeight + Math.round(units.gridUnit / 2)) * 2) + Math.round(units.gridUnit / 4) : 0
            opacity: expanded ? 1 : 0
            highlight: PlasmaComponents.Highlight{}
            delegate: actionItem


            Behavior on opacity { NumberAnimation { duration: units.shortDuration * 3 } }

            Component.onCompleted: currentIndex = -1

            PrintManager.ProcessRunner {
                id: processRunner
            }

            Component {
                id: actionItem

                Item {
                    height: Math.max(actionIcon.height, actionText.height + jobsCountText.height) + Math.round(units.gridUnit / 3)
                    width: actionsList.width

                    PlasmaCore.IconItem {
                        id: actionIcon

                        source: icon
                        height: actionsList.actionIconHeight
                        width: actionsList.actionIconHeight

                        anchors {
                            left: parent.left
                            leftMargin: 3
                            verticalCenter: actionLabels.verticalCenter
                        }
                    }

                    Column {
                        id: actionLabels

                        anchors {
                            left: actionIcon.right
                            leftMargin: 5
                            right: parent.right
                            rightMargin: 3
                            verticalCenter: parent.verticalCenter
                        }

                        PlasmaComponents.Label {
                            id: actionText

                            anchors {
                                left: parent.left
                                right: parent.right
                            }

                            height: paintedHeight
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.WordWrap
                            text: name
                        }

                        PlasmaComponents.Label {
                            id: jobsCountText

                            anchors {
                                left: parent.left
                                right: parent.right
                            }

                            height: actionType == 2 ? paintedHeight : 0
                            elide: Text.ElideRight
                            font.pointSize: theme.smallestFont.pointSize
                            opacity: 0.6
                            text: getJobsLabel()
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true

                        onEntered: {
                            actionsList.currentIndex = index;
                            actionsList.highlightItem.opacity = 1;
                        }
                        onExited: {
                            actionsList.highlightItem.opacity = 0;
                        }
                        onClicked: {
                            // Configure printer
                            if (actionType == 1) {
                                processRunner.configurePrinter(printerName)
                            // Open print queue
                            } else {
                                processRunner.openPrintQueue(printerName)
                            }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        isPaused = printerState === 5
    }

    states: [
        State {
            name: "NORMAL"
            when: !isPaused
        },

        State {
            name: "PAUSED"
            when: isPaused
            PropertyChanges { target: printerNameLabel; opacity: 0.6 }
            PropertyChanges { target: printerIcon; opacity: 0.6 }
        }
    ]

    function getJobsLabel() {
        if (printmanager.jobsFilter == PrintManager.JobModel.WhichActive) {
            if (jobsFilterModel.count == 0) {
                return i18n("No active jobs");
            } else {
                return i18np("One active job", "%1 active jobs", jobsFilterModel.count);
            }
        } else {
            if (jobsFilterModel.count == 0) {
                return i18n("No jobs");
            } else {
                return i18np("One job", "%1 jobs", jobsFilterModel.count);
            }
        }
    }

    function toggleSelection() {
        if (isPaused) {
            if (printerState === 5) {
                printersModel.resumePrinter(printerName)
            }
        } else {
            printersModel.pausePrinter(printerName)
        }
        isPaused = !isPaused
    }
}
