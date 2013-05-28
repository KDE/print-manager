/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2013 by Daniel Nicoletti <dantti12@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import "private" as Private

/**
 * An interactive draggable switch component with Plasma look and feel.
 * Modified as the license above said I can :)
 */
Item {
    id: switcher

    /**
     * type:bool
     *
     * True if the Switch is being pressed.
     */
    property alias pressed: mouseArea.pressed

    // Plasma API
    /**
     * type:bool
     * This property holds if the switch is on or off.
     *
     * The default value is false (which means off).
     */
    property bool on: false

    /**
     * type:bool
     * This property holds if the mouse area of
     * our switch contains the mouse cursor, which
     * is needed to keep the list highlight
     */
    property alias containsMouse: mouseArea.containsMouse

    /**
     * This signal is emitted only when the user
     * manually change the state.
     */
    signal toggled(bool enabled)

    width: handle.width * 2.3
    height: theme.defaultFont.mSize.height*1.8
    opacity: enabled ? 1.0 : 0.5

    Keys.onLeftPressed: {
        if (!enabled)
            return;

        value -= stepSize;
    }

    Keys.onRightPressed: {
        if (!enabled)
            return;

        value += stepSize;
    }

    onOnChanged: updateSwitcher()

    function updateSwitcher() {
        if (switcher.on) {
            handle.x = contents.width - handle.width
        } else {
            handle.x = 0
        }
    }

    Item {
        id: contents

        // Plasma API
        property bool animated: true
        property real handleWidth: grooveSvg.elementSize("horizontal-slider-handle").width
        property real handleHeight: grooveSvg.elementSize("horizontal-slider-handle").height

        width: switcher.width
        height: switcher.height

        anchors.centerIn: parent

        PlasmaCore.Svg {
            id: grooveSvg
            imagePath: "widgets/slider"
        }
        PlasmaCore.FrameSvgItem {
            id: groove
            imagePath: "widgets/slider"
            prefix: "groove"
            height: handle.height * 0.85
            anchors {
                left: parent.left
                right: parent.right
                margins: handle.width / 4
                verticalCenter: parent.verticalCenter
            }
            opacity: switcher.on ? 1 : 0.5
        }
        PlasmaCore.FrameSvgItem {
            id: highlight
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            height: groove.height
            anchors.left: groove.left
            anchors.right: handle.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

        Private.RoundShadow {
            id: shadow
            imagePath: "widgets/slider"
            focusElement: "horizontal-slider-focus"
            hoverElement: "horizontal-slider-hover"
            shadowElement: "horizontal-slider-shadow"
            state: switcher.activeFocus ? "focus" : (mouseArea.containsMouse ? "hover" : "shadow")
            anchors.fill: handle
        }

        PlasmaCore.SvgItem {
            id: handle
            anchors {
                verticalCenter: groove.verticalCenter
            }
            width: contents.handleWidth
            height: contents.handleHeight
            opacity: switcher.enabled ? 1 : 0.5
            svg: PlasmaCore.Svg { imagePath: "widgets/slider" }
            elementId: "horizontal-slider-handle"

            Behavior on x {
                id: behavior
                enabled: !mouseArea.drag.active && contents.animated

                PropertyAnimation {
                    duration: behavior.enabled ? 150 : 0
                    easing.type: Easing.OutSine
                }
            }

            Rectangle {
                property bool showOn: handle.x >= (contents.width - handle.width) / 2
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: showOn ? 1.5 : height
                color: showOn ? "dimgrey" : "transparent"
                border.width: showOn ? 0 : 2
                border.color: "dimgrey"
                radius: showOn ? 0 : height
                smooth: true
            }
        }

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            enabled: switcher.enabled
            drag {
                target: handle
                axis: Drag.XAxis
                minimumX: 0 //range.positionAtMinimum
                maximumX: contents.width - handle.width //range.positionAtMaximum
            }
            hoverEnabled: true

            onClicked: {
                switcher.on = !switcher.on
                toggled(switcher.on)
            }

            onPressed: switcher.forceActiveFocus()
            onReleased: {
                var xValue = handle.x
                var onBefore = switcher.on
                switcher.on = xValue >= (contents.width - handle.width) / 2
                if (onBefore === switcher.on && (xValue !== contents.width - handle.width || xValue === 0)) {
                    updateSwitcher()
                } else if (onBefore !== switcher.on) {
                    toggled(switcher.on)
                }
            }
        }
    }
}
