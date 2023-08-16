/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 2.9

import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.printmanager 0.2 as PrintManager

PlasmaExtras.ExpandableListItem {
    readonly property bool isPaused: model.printerState === 5

    icon: model.iconName
    iconEmblem: isPaused ? "emblem-pause" : ""
    title: model.printerName
    subtitle: model.stateMessage
    isDefault: model.isDefault
    defaultActionButtonAction: Action {
        icon.name: isPaused ? "media-playback-start" : "media-playback-pause"
        text: isPaused ? i18n("Resume printing") : i18n("Pause printing")
        onTriggered: {
            if (isPaused) {
                printersModel.resumePrinter(model.printerName);
            } else {
                printersModel.pausePrinter(model.printerName);
            }
        }
    }
    contextualActionsModel: [
        Action {
            icon.name: "configure"
            text: i18n("Configure printer...")
            onTriggered: processRunner.configurePrinter(model.printerName);
        },
        Action {
            icon.name: "view-list-details"
            text: i18n("View print queue")
            onTriggered: processRunner.openPrintQueue(printerName);
        }
    ]
}
