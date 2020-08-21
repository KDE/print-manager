/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   Copyright 2014-2015 Jan Grulich <jgrulich@redhat.com>
 *   Copyright 2020 Nate Graham <nate@kde.org>
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
