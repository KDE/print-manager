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
    id: jobItem
    width: jobItem.ListView.view.width
    height: 30

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
        Behavior on opacity { PropertyAnimation {} }
        anchors.fill: parent
        
        Component.onCompleted: {
            // Try to avoid calling jobsView directly
            jobsView.highlight.connect(highlightJobs)
        }
        function highlightJobs(printer) {
            if (printer == jobPrinter) {
                padding.opacity = 0.7;
            } else {
                padding.opacity = 0;
            }
        }
        
        PlasmaCore.ToolTip {
            target: parent
            mainText: i18n("Job: %1", jobId)
            // TODO completedAt
            subText: i18n("Owner: %1<br>Size: %2<br>Created At: %3", jobOwner, jobSize, jobCreatedAt)
        }
    }
    
    MouseArea {
        id: container
        anchors {
            fill: parent
            topMargin: padding.margins.top
            leftMargin: padding.margins.left
            rightMargin: padding.margins.right
            bottomMargin: padding.margins.bottom
        }
        hoverEnabled: true
        onEntered: {
            padding.opacity = 0.7;
        }
        onClicked: {
            jobItem.ListView.view.currentIndex = index;
        }
        onExited: {
            padding.opacity = 0;
        }
        
        Row {
            spacing: 4
            anchors {
                left: parent.left
                right: parent.right
            }

            QIconItem {
                id: jobIcon
                width: 16
                height: 16
                icon: QIcon("media-playback-pause")
            }
            PlasmaComponents.Label {
                // 12 = 3 * spacing
                width: parent.width - pagesLabel.width - jobIcon.width - 12
                height: paintedHeight
                elide: Text.ElideRight
                text: jobName
            }
            PlasmaComponents.Label {
                id: pagesLabel
                horizontalAlignment: "AlignRight"
                height: paintedHeight
                width: paintedWidth
                text: jobPages
                visible: jobPages != 0
                font.pointSize: theme.smallestFont.pointSize
                color: "#99"+(theme.textColor.toString().substr(1))
            }
        }
    }
}