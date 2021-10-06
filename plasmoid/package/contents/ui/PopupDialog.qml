/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents // for Highlight
import org.kde.plasma.printmanager 0.2 as PrintManager

FocusScope {
    id: dialog

    property bool scrollBarVisible: printersView.contentHeight > scrollArea.height
    property bool searchBarVisible: scrollBarVisible || searchBar.text.length !== 0


    TextField {
        id: searchBar

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            rightMargin: Math.round(units.gridUnit / 2)
        }

        visible: searchBarVisible
        placeholderText: i18n("Search for a printer...")

        onTextChanged: {
            printersFilterModel.setFilterWildcard(text)
        }
    }

    PrintManager.ProcessRunner {
        id: processRunner
    }

    PrintManager.PrinterSortFilterModel {
        id: printersFilterModel
        sourceModel: printersModel
    }

    PlasmaExtras.ScrollArea {
        id: scrollArea
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            top: searchBarVisible ? searchBar.bottom : parent.top
        }

        ListView {
            id: printersView

            anchors.fill: parent
            focus: true
            currentIndex: -1
            clip: true
            model: printersFilterModel
            highlight: PlasmaComponents.Highlight{ }
            highlightMoveDuration: 0
            highlightResizeDuration: 0
            delegate: PrinterItem {
                width: printersView.width
            }

            PlasmaExtras.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (units.largeSpacing * 4)

                visible: printersFilterModel.count === 0 || serverUnavailable
                text: serverUnavailable ?
                        printersModelError :
                        i18n("No printers have been configured or discovered")
                iconName: serverUnavailable ? "dialog-error" : ""
            }
        }
    }
}
