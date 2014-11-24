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
import org.kde.printmanager 0.1 as PrintManager

FocusScope {
    id: dialog

    property string printersModelError: ""
    property alias serverUnavailable: printersModel.serverUnavailable

    state: "NO_PRINTER"



//     PlasmaExtras.ScrollArea {
//         id: scrollView;
//
//         anchors.fill: parent
//         opacity: 0

        ListView {
            id: printersView

            anchors.fill: parent
            opacity: 0
            focus: true
            currentIndex: -1
            clip: true;
            boundsBehavior: Flickable.StopAtBounds
            model: PrintManager.PrinterSortFilterModel {
                id: printersFilterModel
                sourceModel: PrintManager.PrinterModel {
                    id: printersModel
                    onCountChanged: updatePrinterStatus()
                    onError: printersModelError = errorTitle
                }
            }
            onCountChanged: updatePrinterStatus()
            delegate: PrinterItem { }
        }
//     }

    StatusView {
        id: statusNoPrinter
        anchors.fill: parent
        opacity: 0
        iconName: serverUnavailable ? "dialog-error" : "dialog-information"
        title: serverUnavailable ?
                   printersModelError :
                   i18n("No printers have been configured or discovered")
    }

    states: [
        State {
            name: "NO_PRINTER"
            PropertyChanges { target: statusNoPrinter; opacity: 1 }
            PropertyChanges { target: printmanager; tooltipText: statusNoPrinter.title }
        },
        State {
            name: "JOBS_PRINTER"
//             PropertyChanges { target: scrollView; opacity: 1 }
            PropertyChanges { target: printersView; opacity: 1 }
            PropertyChanges { target: printmanager; tooltipText: jobsTooltipText }
        }
    ]

    function updatePrinterStatus() {
        var printersFilterCount = printersFilterModel.count;
        if (printersFilterCount >= 1) {
            dialog.state = "JOBS_PRINTER";
        } else {
            dialog.state = "NO_PRINTER";
        }
    }
}
