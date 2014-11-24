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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

PlasmaComponents.ListItem {
    id: printerItem

    property bool expanded: false
    property bool isPaused: false

    height: expanded ? printerBaseItem.height + expandableComponentLoader.height + Math.round(units.gridUnit / 3) : printerBaseItem.height
//     checked: ListView.isCurrentItem
    enabled: true

    function toggleSelection() {
        if (isPaused) {
//             if (!isAcceptingJobs) {
//                 printersModel.acceptJobs(printerName)
//             }
            if (printerState === 5) {
                printersModel.resumePrinter(printerName)
            }
        } else {
            printersModel.pausePrinter(printerName)
        }
        isPaused = !isPaused
    }

    Item {
        id: printerBaseItem

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            // Reset top margin from PlasmaComponents.ListItem
            topMargin: -Math.round(units.gridUnit / 3)
        }

        height: Math.max(units.iconSizes.medium, printerNameLabel.height + printerStatusLabel.height)  + Math.round(units.gridUnit / 2)

        KQuickControlsAddons.QIconItem {
            id: printerIcon

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }

            height: units.iconSizes.medium
            width: height
            icon: iconName
            Behavior on opacity { PropertyAnimation {} }
        }

        PlasmaComponents.Label {
            id: printerNameLabel

            anchors {
                bottom: printerIcon.verticalCenter
                left: printerIcon.right
                leftMargin: Math.round(units.gridUnit / 2)
                right: stateChangeButton.visible ? stateChangeButton.left : parent.right
            }

            height: paintedHeight
            elide: Text.ElideRight
            text: printerName
        }

        PlasmaComponents.Label {
            id: printerStatusLabel

            anchors {
                left: printerIcon.right
                leftMargin: Math.round(units.gridUnit / 2)
                right: stateChangeButton.visible ? stateChangeButton.left : parent.right
                top: printerNameLabel.bottom
            }

            height: paintedHeight
            elide: Text.ElideRight
            font.pointSize: theme.smallestFont.pointSize
            opacity: 0.6
            text: stateMessage
        }

        PlasmaComponents.ToolButton {
            id: stateChangeButton

            anchors {
                right: parent.right
                rightMargin: Math.round(units.gridUnit / 2)
                verticalCenter: printerIcon.verticalCenter
            }

            iconSource: isPaused ? "media-playback-start" : "media-playback-pause"
            opacity: printerItem.containsMouse ? 1 : 0
            visible: opacity != 0

            onClicked: toggleSelection()
        }
    }

    Loader {
        id: expandableComponentLoader

        anchors {
            left: parent.left
            right: parent.right
            top: printerBaseItem.bottom
        }
    }

    Component {
        id: detailsComponent

        Item {
            height: childrenRect.height

            PlasmaExtras.ScrollArea {
                id: scrollView

                width: parent.width
                height: printerItem.ListView.view.height - printerBaseItem.height

                ListView {
                    id: jobsView

                    anchors.fill: parent

                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    model: jobsFilterModel
                    delegate: JobItem {
                        onClicked: {
                            printerItem.expanded = false
                            ListView.view.currentIndex = -1
                        }
                    }

                    Component.onCompleted: {
                        jobsFilterModel.filteredPrinters = printerName
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "NORMAL"
            when: !isPaused && !expanded
            StateChangeScript { script: if (expandableComponentLoader.status == Loader.Ready) {expandableComponentLoader.sourceComponent = undefined} }
        },

        State {
            name: "PAUSED"
            when: isPaused && !expanded
            PropertyChanges { target: printerNameLabel; opacity: 0.6 }
            PropertyChanges { target: printerIcon; opacity: 0.6 }
        },

        State {
            name: "EXPANDED"
            when: !isPaused && expanded
            PropertyChanges { target: printerItem.ListView.view; interactive: false }
            StateChangeScript { script: createContent(); }
        },

        State {
            name: "PAUSED_EXPANDED"
            when: isPaused && expanded
            PropertyChanges { target: printerNameLabel; opacity: 0.6 }
            PropertyChanges { target: printerIcon; opacity: 0.6 }
            PropertyChanges { target: printerItem.ListView.view; interactive: false }
            StateChangeScript { script: createContent(); }
        }
    ]

    onStateChanged: {
        if (state == "EXPANDED" || state == "PAUSED_EXPANDED") {
            ListView.view.currentIndex = index
        }
    }

    onClicked: {
        expanded = !expanded

        if (!expanded) {
            ListView.view.currentIndex = -1
            jobsFilterModel.filteredPrinters = ""
        }
    }

    function createContent() {
        expandableComponentLoader.sourceComponent = detailsComponent
    }
}
