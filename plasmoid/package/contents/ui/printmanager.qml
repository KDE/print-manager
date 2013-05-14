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

Item {
    id: printmanager
    property int minimumWidth: horizontalLayout ? 500 : 300
    property int minimumHeight: 270
    
    property string highlightPrinter
    property bool horizontalLayout: false
    property string filterPrinters

    PlasmaCore.Theme {
        id: theme
    }

    Component.onCompleted: {
        // This allows the plasmoid to shrink when the layout changes
        plasmoid.aspectRatioMode = IgnoreAspectRatio
        plasmoid.addEventListener('ConfigChanged', configChanged);
        plasmoid.popupEvent.connect(popupEventSlot);
        configChanged();

        checkPlasmoidStatus();
    }

    function configChanged() {
        var completedJobs = plasmoid.readConfig("completedJobs");
        var allJobs = plasmoid.readConfig("allJobs");
        if (completedJobs == true) {
            jobsModel.setWhichJobs(PrintManager.PrintQueueModel.WhichCompleted);
        } else if (allJobs == true) {
            jobsModel.setWhichJobs(PrintManager.PrintQueueModel.WhichAll);
        } else {
            jobsModel.setWhichJobs(PrintManager.PrintQueueModel.WhichActive);
        }

        if (plasmoid.readConfig("filterPrinters") == true) {
            filterPrinters = plasmoid.readConfig("selectedPrinters");
            console.debug("selectedPrinters: " + filterPrinters)
        } else {
            filterPrinters = "";
        }

        printersView.currentIndex = -1;
        jobsView.currentIndex = -1;
    }

    function checkPlasmoidStatus() {
        var data = new Object
        data["image"] = "printer"
        data["subText"] = "ToolTip descriptive sub text"
        if (jobsFilterModel.count === 0) {
            plasmoid.status = "PassiveStatus";
            data["subText"] = i18n("Print queue is empty");
        } else {
            plasmoid.status = "ActiveStatus"
            data["subText"] = i18np("There is one print job in the queue",
                                    "There are %1 print jobs in the queue",
                                    jobsFilterModel.count);

        }
        plasmoid.popupIconToolTip = data
    }

    function popupEventSlot(popped) {
        if (!popped) {
            checkPlasmoidStatus();
            printersView.currentIndex = -1;
            jobsView.currentIndex = -1;
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
            width:  horizontalLayout ? parent.width * 0.40 - headerSeparator.width : parent.width
            height: horizontalLayout ? parent.height : 50
            currentIndex: -1
            model: PrintManager.PrinterSortFilterModel {
                id: printersFilterModel
                sourceModel: PrintManager.PrinterModel {
                    id: printersModel
                }
                filteredPrinters: filterPrinters
            }
            delegate: PrinterItem{
                multipleItems: horizontalLayout
            }
            interactive: horizontalLayout
            onCountChanged: horizontalLayout = printersFilterModel.count > 1
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
            width:  horizontalLayout ? parent.width * 0.60 - headerSeparator.width : parent.width
            height: horizontalLayout ? parent.height : printmanager.height - headerSeparator.height - printersView.height
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
            onCountChanged: {
                checkPlasmoidStatus();
            }
            delegate: JobItem{}
        }
    }
}
