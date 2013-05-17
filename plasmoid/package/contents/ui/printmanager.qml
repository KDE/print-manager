/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.printmanager 0.1 as PrintManager

FocusScope   {
    id: printmanager
    property int minimumWidth: horizontalLayout ? 500 : 300
    property int minimumHeight: 270
    
    property string highlightPrinter
    property bool horizontalLayout: false
    property string filterPrinters

    property Component compactRepresentation: CompactRepresentation {
        jobsActiveCount: jobsView.view.count
    }

    PlasmaCore.Theme {
        id: theme
    }

    onHorizontalLayoutChanged: {
        if (horizontalLayout === false) {
            printersView.currentIndex = -1;
        }
    }

    Component.onCompleted: {
        // This allows the plasmoid to shrink when the layout changes
        plasmoid.aspectRatioMode = IgnoreAspectRatio
        plasmoid.addEventListener('ConfigChanged', configChanged);
        plasmoid.popupEvent.connect(popupEventSlot);
        configChanged();
    }

    function configChanged() {
        printersView.currentIndex = -1;
//        jobsView.currentIndex = -1;

        if (plasmoid.readConfig("completedJobs") == true) {
            jobsModel.setWhichJobs(PrintManager.PrintQueueModel.WhichCompleted);
        } else if (plasmoid.readConfig("allJobs") == true) {
            jobsModel.setWhichJobs(PrintManager.PrintQueueModel.WhichAll);
        } else {
            jobsModel.setWhichJobs(PrintManager.PrintQueueModel.WhichActive);
        }

        if (plasmoid.readConfig("filterPrinters") == true) {
            filterPrinters = plasmoid.readConfig("selectedPrinters");
        } else {
            filterPrinters = "";
        }
    }

    function popupEventSlot(popped) {
        if (popped) {
            printmanager.forceActiveFocus();
//            printersView.currentIndex = -1;
//            jobsView.currentIndex = -1;
        }
    }

    Column {
        id: columnLayout
        spacing: 2
        anchors.fill: parent

        state: horizontalLayout ? "horizontal" : "vertical"
        states: [
            State {
                name: "vertical"
                ParentChange { target: printersView; parent: columnLayout }
                ParentChange { target: headerSeparator; parent: columnLayout }
                ParentChange { target: jobsView; parent: columnLayout }
            },
            State {
                name: "horizontal"
                ParentChange { target: printersView; parent: rowLayout }
                ParentChange { target: headerSeparator; parent: rowLayout }
                ParentChange { target: jobsView; parent: rowLayout }
            }
        ]
    }
    Row {
        id: rowLayout
        spacing: 2
        anchors.fill: parent
        ScrollableListView {
            id: printersView
            focus: true
            width:  horizontalLayout ? parent.width * 0.40 - headerSeparator.width : parent.width
            height: horizontalLayout ? parent.height : 50
            KeyNavigation.tab: jobsView
            KeyNavigation.backtab: jobsView
            currentIndex: -1
            model: PrintManager.PrinterSortFilterModel {
                id: printersFilterModel
                sourceModel: PrintManager.PrinterModel {
                    id: printersModel
                }
                filteredPrinters: filterPrinters
            }
            delegate: PrinterItem {
                multipleItems: horizontalLayout
            }
            onCountChanged: {
                if (printersFilterModel.count > 1) {
                    horizontalLayout = true;
                } else if (printersFilterModel.count === 1) {
                    horizontalLayout = false;
                }
            }
        }
        
        PlasmaCore.SvgItem {
            id: headerSeparator
            svg: PlasmaCore.Svg {
                id: lineSvg
                imagePath: "widgets/line"
            }
            elementId: horizontalLayout ? "vertical-line" : "horizontal-line"
            height:    horizontalLayout ? parent.height : lineSvg.elementSize("horizontal-line").height
            width:     horizontalLayout ? lineSvg.elementSize("vertical-line").width : parent.width
        }

        ScrollableListView {
            id: jobsView
            width:  horizontalLayout ? parent.width * 0.6 - headerSeparator.width : parent.width
            height: horizontalLayout ? parent.height : printmanager.height - headerSeparator.height - printersView.height
            KeyNavigation.tab: printersView
            KeyNavigation.backtab: printersView
            currentIndex: -1
            model: PlasmaCore.SortFilterModel {
                id: jobsFilterModel
                sourceModel: PrintManager.PrintQueueModel {
                    id: jobsModel
                }
                filterRole: "jobPrinter"
                filterRegExp: filterPrinters
                sortRole: "jobId"
                sortOrder: Qt.AscendingOrder
            }
            delegate: JobItem {}
        }
    }
}
