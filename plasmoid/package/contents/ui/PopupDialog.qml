/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.printmanager 0.2 as PrintManager

PlasmaExtras.Representation {
    id: dialog

    collapseMarginsHint: true

    header: PlasmaExtras.PlasmoidHeading {
        TextField {
            id: searchBar
            anchors.fill: parent

            placeholderText: i18n("Search for a printer...")

            onTextChanged: {
                printersFilterModel.setFilterWildcard(text)
            }
        }
    }

    PrintManager.ProcessRunner {
        id: processRunner
    }

    PlasmaComponents3.ScrollView {
        anchors.fill: parent

        // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
        PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

        contentWidth: availableWidth - contentItem.leftMargin - contentItem.rightMargin

        contentItem: ListView {
            id: printersView

            focus: true
            currentIndex: -1
            model: PrintManager.PrinterSortFilterModel {
                id: printersFilterModel
                sourceModel: printersModel
            }
            topMargin: PlasmaCore.Units.smallSpacing * 2
            bottomMargin: PlasmaCore.Units.smallSpacing * 2
            leftMargin: PlasmaCore.Units.smallSpacing * 2
            rightMargin: PlasmaCore.Units.smallSpacing * 2
            spacing: PlasmaCore.Units.smallSpacing

            highlight: PlasmaExtras.Highlight {}
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            delegate: PrinterItem {}

            PlasmaExtras.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (PlasmaCore.Units.largeSpacing * 4)

                visible: printersFilterModel.count === 0 || serverUnavailable
                text: serverUnavailable ?
                        printersModelError :
                        i18n("No printers have been configured or discovered")
                iconName: serverUnavailable ? "dialog-error" : ""
            }
        }
    }
}
