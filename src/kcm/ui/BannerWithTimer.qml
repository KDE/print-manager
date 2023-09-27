/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.InlineMessage {
    id: banner
    type: Kirigami.MessageType.Error
    showCloseButton: true

    property bool resetToDefault: true
    property int defaultType: Kirigami.MessageType.Error
    property alias autoCloseInterval: timer.interval

    signal timeout()

    onVisibleChanged: {
        if (visible && timer.interval > 0) {
            timer.start()
        }
    }

    Timer {
        id: timer
        repeat: false; interval: 10000
        onTriggered: {
            banner.visible = false
            if (banner.resetToDefault) {
                banner.type = defaultType
            }
            timeout()
        }
    }
}

