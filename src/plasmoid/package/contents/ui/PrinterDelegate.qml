/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PrintManager

PlasmaExtras.ExpandableListItem {
    readonly property bool isPaused: model.printerState === 5
    readonly property bool discovered: model.uriSupported === "_discovered"

    icon: {
        if (model.remote) {
            return "folder-network-symbolic"
        } else if (model.isClass) {
            return "folder-print"
        } else {
            return discovered
                ? "view-web-browser-dom-tree-symbolic"
                : model.iconName
        }
    }
    iconEmblem: isPaused ? "emblem-pause" : ""
    title: model.info + (model.location && printersModel.displayLocationHint
            ? " (%1)".arg(model.location)
            : "")
    subtitle: model.stateMessage
    isDefault: model.isDefault

    defaultActionButtonAction: Kirigami.Action {
        icon.name: isPaused ? "media-playback-start" : "media-playback-pause"
        text: isPaused ? i18n("Resume") : i18n("Pause")
        enabled: !discovered
        onTriggered: {
            if (isPaused) {
                printersModel.resumePrinter(model.printerName);
            } else {
                printersModel.pausePrinter(model.printerName);
            }
        }
    }

    contextualActions: [
        Kirigami.Action {
            icon.name: "configure"
            text: i18n("Configure printer…")
            enabled: !discovered
            onTriggered: PrintManager.ProcessRunner.configurePrinter(model.printerName);
        },
        Kirigami.Action {
            icon.name: "view-list-details"
            text: i18n("View print queue…")
            enabled: !discovered
            onTriggered: PrintManager.ProcessRunner.openPrintQueue(model.printerName);
        }
    ]
}
