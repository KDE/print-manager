/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami 2 as Kirigami
import org.kde.plasma.printmanager as PrintManager

PlasmaExtras.Representation {
    collapseMarginsHint: true

    header: PlasmaExtras.PlasmoidHeading {
        Kirigami.SearchField {
            anchors.fill: parent
            onAccepted: printersFilterModel.setFilterWildcard(text)
        }
    }

    PrintManager.ProcessRunner {
        id: processRunner
    }
   
    PlasmaComponents3.ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

        contentItem: ListView {
            focus: true
            currentIndex: -1

            model: PrintManager.PrinterSortFilterModel {
                id: printersFilterModel
                sourceModel: printersModel
            }
            
            topMargin: Kirigami.Units.smallSpacing * 2
            bottomMargin: Kirigami.Units.smallSpacing * 2
            leftMargin: Kirigami.Units.smallSpacing * 2
            rightMargin: Kirigami.Units.smallSpacing * 2
            spacing: Kirigami.Units.smallSpacing

            highlight: PlasmaExtras.Highlight {}
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            delegate: PrinterDelegate {}

            Loader {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                active: printersFilterModel.count === 0 || serverUnavailable
                sourceComponent: PlasmaExtras.PlaceholderMessage {
                    text: serverUnavailable ? printersModelError || i18n("No printers have been configured or discovered") : i18n("No matches")
                    iconName: serverUnavailable ? "dialog-error" : "edit-none"
                }
            }
        }
    }
}
