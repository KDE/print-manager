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
    state: "NO_PRINTER"

    property int minimumWidth: horizontalLayout ? 500 : 300
    property int minimumHeight: 270
    
    property string highlightPrinter
    property bool horizontalLayout: false
    property string filterPrinters
    property string tooltipText
    property string plasmoidStatus: "PassiveStatus"
    property string jobsTooltipText
    property string printersModelError: ""
    property alias serverUnavailable: printersModel.serverUnavailable

    property Component compactRepresentation: CompactRepresentation {
        tooltipText: printmanager.tooltipText
    }

    PlasmaCore.Theme {
        id: theme
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
        jobsView.currentIndex = -1;

        if (plasmoid.readConfig("completedJobs") == true) {
            jobsModel.setWhichJobs(PrintManager.JobModel.WhichCompleted);
        } else if (plasmoid.readConfig("allJobs") == true) {
            jobsModel.setWhichJobs(PrintManager.JobModel.WhichAll);
        } else {
            jobsModel.setWhichJobs(PrintManager.JobModel.WhichActive);
        }

        if (plasmoid.readConfig("filterPrinters") == true) {
            filterPrinters = plasmoid.readConfig("selectedPrinters");
        } else {
            filterPrinters = "";
        }

        updateJobStatus();
        updatePrinterStatus();
    }

    function updateJobStatus() {
        var activeCount = jobsFilterModel.activeCount;
        if (activeCount === 0) {
            plasmoidStatus = "PassiveStatus";
            jobsTooltipText = i18n("Print queue is empty");
        } else {
            plasmoidStatus = "ActiveStatus";
            jobsTooltipText = i18np("There is one print job in the queue",
                                    "There are %1 print jobs in the queue",
                                    activeCount);
        }
    }

    function updatePrinterStatus() {
        var printersFilterCount = printersFilterModel.count;
        if (printersFilterCount > 1) {
            horizontalLayout = true;
            printmanager.state = "JOBS_PRINTER";
        } else if (printersFilterCount === 1) {
            horizontalLayout = false;
            printmanager.state = "JOBS_PRINTER";
        } else if (printersFilterCount === 0 &&
                   printersModel.count > 0) {
            printmanager.state = "PRINTER_FILTER";
        } else {
            printmanager.state = "NO_PRINTER";
        }
    }

    function popupEventSlot(popped) {
        if (popped) {
            printmanager.forceActiveFocus();
            printersView.currentIndex = -1;
            jobsView.currentIndex = -1;
        } else {
            updateJobStatus();
        }
    }

    Column {
        id: columnLayout
        spacing: 2
        anchors.fill: parent
        opacity: 0

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
        opacity: 0
        ListView {
            id: printersView
            focus: true
            width:  horizontalLayout ? parent.width * 0.5 - headerSeparator.width : parent.width
            height: horizontalLayout ? parent.height : 50
            KeyNavigation.tab: jobsView
            KeyNavigation.backtab: jobsView
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            model: PrintManager.PrinterSortFilterModel {
                id: printersFilterModel
                sourceModel: PrintManager.PrinterModel {
                    id: printersModel
                    onCountChanged: updatePrinterStatus()
                    onError: printersModelError = errorTitle
                }
                filteredPrinters: filterPrinters
            }
            onCountChanged: updatePrinterStatus()
            delegate: PrinterItem {
                multipleItems: horizontalLayout
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
            width:  horizontalLayout ? parent.width * 0.5 - headerSeparator.width : parent.width
            height: horizontalLayout ? parent.height : printmanager.height - headerSeparator.height - printersView.height
            KeyNavigation.tab: printersView
            KeyNavigation.backtab: printersView
            currentIndex: -1
            model: PrintManager.JobSortFilterModel {
                id: jobsFilterModel
                sourceModel: PrintManager.JobModel {
                    id: jobsModel
                }
                filteredPrinters: filterPrinters
                onActiveCountChanged: updateJobStatus()
            }
            delegate: JobItem {}
        }
    }

    StatusView {
        id: statusNoPrinter
        anchors.fill: parent
        opacity: 0
        iconName: serverUnavailable ? "dialog-error" : "dialog-information"
        title: serverUnavailable ?
                   printersModelError :
                   i18n("No printers have been configured or discovered")
    }

    StatusView {
        id: statusPrinterFilter
        anchors.fill: parent
        opacity: 0
        iconName: "dialog-information"
        title: i18n("There is currently no available printer matching the selected filters")
    }

    states: [
        State {
            name: "NO_PRINTER"
            PropertyChanges { target: statusNoPrinter; opacity: 1 }
            PropertyChanges { target: printmanager; tooltipText: statusNoPrinter.title }
            PropertyChanges { target: plasmoid; status: "PassiveStatus" }
        },
        State {
            name: "PRINTER_FILTER"
            PropertyChanges { target: statusPrinterFilter; opacity: 1 }
            PropertyChanges { target: printmanager; tooltipText: statusPrinterFilter.title }
            PropertyChanges { target: plasmoid; status: "PassiveStatus" }
        },
        State {
            name: "JOBS_PRINTER"
            PropertyChanges { target: columnLayout; opacity: 1 }
            PropertyChanges { target: rowLayout; opacity: 1 }
            PropertyChanges { target: printmanager; tooltipText: jobsTooltipText }
            PropertyChanges { target: plasmoid; status: plasmoidStatus }
        }
    ]
}
