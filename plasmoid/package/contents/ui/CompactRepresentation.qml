/*
 *   Copyright 2013 Daniel Nicoletti <dantti12@gmail.com>
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
import org.kde.qtextracomponents 0.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: panelIconWidget;

    property int jobsActiveCount: 0

    onJobsActiveCountChanged: {
        if (jobsActiveCount === 0) {
            plasmoid.status = "PassiveStatus";
            tooltip.subText = i18n("Print queue is empty");
        } else {
            plasmoid.status = "ActiveStatus"
            tooltip.subText = i18np("There is one print job in the queue",
                                    "There are %1 print jobs in the queue",
                                    jobsActiveCount);

        }
    }

    PlasmaCore.SvgItem {
        id: connectionIcon
        anchors.fill: parent
        svg: PlasmaCore.Svg {
            imagePath: "icons/printer";
        }
        elementId: "printer";
    }

    MouseArea {
        id: mouseAreaPopup

        anchors.fill: parent
        hoverEnabled: true
        onClicked: plasmoid.togglePopup()

        PlasmaCore.ToolTip {
             id: tooltip
             target: mouseAreaPopup
             image: QIcon("printer")
        }
    }
}
