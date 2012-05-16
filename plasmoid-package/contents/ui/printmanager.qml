/*
 *   Copyright 2012 Daniel Nicoletti <dantti12@gmail.com>
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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: printmanager
    property int minimumWidth: horizontalLayout ? 500 : 300
    property int minimumHeight: 270
    
    property string highlightPrinter
    property bool horizontalLayout: printersFilterModel.count > 1
    property string whichPrinter

    PlasmaCore.Theme {
        id: theme
    }
    
    Component.onCompleted: {
        // This allows the plasmoid to shrink when the layout changes
        plasmoid.aspectRatioMode = IgnoreAspectRatio
        plasmoid.addEventListener ('ConfigChanged', configChanged);
        plasmoid.popupEvent.connect(popupEventSlot);
        configChanged();
        
        checkPlasmoidStatus();
    }
    
    function configChanged() {
        whichPrinter = plasmoid.readConfig("printerName");

        printersView.currentIndex = -1;
        jobsView.currentIndex = -1;
    }
    
    function checkPlasmoidStatus() {
        if (jobsFilterModel.count == 0) {
            plasmoid.status = "PassiveStatus"
        } else {
            plasmoid.status = "ActiveStatus"
        }
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
            model: PlasmaCore.SortFilterModel {
                id: printersFilterModel
                sourceModel: PlasmaCore.DataModel {
                    id: printersModel
                    dataSource: PlasmaCore.DataSource {
                        engine: "org.kde.printers"
                        connectedSources: sources
                        interval: 0
                    }
                }
                filterRole: "printerName"
                filterRegExp: whichPrinter
                sortRole: "info"
                sortOrder: Qt.AscendingOrder
            }
            delegate: PrinterItem{
                multipleItems: horizontalLayout
            }
            interactive: horizontalLayout
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
            model: PlasmaCore.SortFilterModel {
                id: jobsFilterModel
                sourceModel: PlasmaCore.DataModel {
                    id: jobsModel
                    dataSource: PlasmaCore.DataSource {
                        engine: "org.kde.printjobs"
                        connectedSources: sources
                        interval: 0
                    }
                }
                filterRole: "jobPrinter"
                filterRegExp: whichPrinter
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
