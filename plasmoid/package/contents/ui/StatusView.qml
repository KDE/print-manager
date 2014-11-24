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
