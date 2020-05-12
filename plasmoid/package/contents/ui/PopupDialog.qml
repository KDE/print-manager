/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   Copyright 2014-2015 Jan Grulich <jgrulich@redhat.com>
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
import QtQuick.Controls 1.3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.printmanager 0.2 as PrintManager

import org.kde.kirigami 2.12 as Kirigami

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
            highlightMoveDuration: units.longDuration
            highlightResizeDuration: units.longDuration
            delegate: PrinterItem { }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (units.largeSpacing * 4)

                visible: printersFilterModel.count === 0 || serverUnavailable
                text: serverUnavailable ?
                        printersModelError :
                        i18n("No printers have been configured or discovered")
                icon.name: serverUnavailable ? "dialog-error" : undefined
            }
        }
    }
}
