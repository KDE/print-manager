/*
 * Copyright 2012  Daniel Nicoletti <dantti12@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Item {
    id: statusView

    property alias title: titleText.text
    property string iconName: ""
    property int iconSize: 128
    property alias preferedHeight: column.height

    Column {
        id: column
        width: parent.width
        anchors.centerIn: parent
        spacing: 4
        Item {
            id: image
            width: iconSize
            height: iconSize
            anchors.horizontalCenter: parent.horizontalCenter

            KQuickControlsAddons.QIconItem {
                id: statusIcon
                anchors.fill: parent
                icon: iconName
            }
        }
        PlasmaComponents.Label {
            id: titleText
            width: parent.width
            height: paintedHeight
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
