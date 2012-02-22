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
    property int minimumWidth: horizontalLayout ? 650 : 300
    property int minimumHeight: 270
    
    property bool horizontalLayout: printersModel.count > 1
    property string whichJobs
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
    }
    
    function configChanged() {
        var activeJobs = plasmoid.readConfig("activeJobs");
        var allJobs = plasmoid.readConfig("allJobs");
        var _whichJobs;
        if (activeJobs == true) {
            _whichJobs = "ActiveJobs";
        } else if (allJobs == true) {
            _whichJobs = "AllJobs";
        } else {
            _whichJobs = "CompletedJobs";
        }
        
        if (_whichJobs != whichJobs) {
            console.debug("--------Which JOBS Changed:" + whichJobs + " to " + _whichJobs);
            whichJobs = _whichJobs;
            jobsSource.connectedSources = [whichJobs];
            jobsSource.onCompleted(sources);
            console.debug("--------Which JOBS sources:" + jobsSource.sources);
        }

        printersView.currentIndex = -1;
        jobsView.currentIndex = -1;
    }
    
    function popupEventSlot(popped) {
        if (!popped) {
            printersView.currentIndex = -1;
            jobsView.currentIndex = -1;
        }
    }

    PlasmaCore.DataSource {
        id: printersSource
        engine: "printmanager"
        connectedSources: ["Printers"]
        interval: 1500
        
        onSourceAdded: {
            console.debug(source);
            var pattern = /Printers(\/[^\/]+)?$/
            if (source.match(pattern)) {
                console.debug("Source Connected:" + source);
                connectSource(source);
            }            
        }
        Component.onCompleted: {
            var pattern = /Printers(\/[^\/]+)?$/
            for (var i in sources) {
                console.debug("COMPLETED " + sources[i]);
                if (sources[i].match(pattern)) {
                    console.debug("COMPLETED Source Connected:" + sources[i]);
                    connectSource(sources[i]);
                }
            }
        }
    }
    
    PlasmaCore.DataSource {
        id: jobsSource
        engine: "printmanager"
//         connectedSources: whichJobs
        interval: 1500
        
        onSourceAdded: {
            console.debug("Job onSourceAdded " + source);
//             list.onCountChanged: {
//                 if (count == 0) {
//                     plasmoid.status = "PassiveStatus"
//                 } else {
//                     plasmoid.status = "PassiveStatus"
//                 }
//             }
            
            var re = new RegExp(whichJobs + "(/\\d+)?$");
            if (source.match(re)) {
                console.debug("Job onSourceAdded =  Source Connected:" + source);
                connectSource(source);
            }            
        }
        Component.onCompleted: {
            var re = new RegExp(whichJobs + "(/\\d+)?$")
            for (var i in sources) {
                console.debug("Job onCOMPLETED " + sources[i]);
                if (sources[i].match(re)) {
                    console.debug("Job onCOMPLETED Source Connected:" + sources[i]);
                    connectSource(sources[i]);
                }
            }
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
            signal highlight(string printer)
            width:  horizontalLayout ? parent.width * 0.40 - headerSeparator.width : parent.width
            height: horizontalLayout ? parent.height : 50
            currentIndex: -1
            model: PlasmaCore.DataModel {
                id: printersModel
                dataSource: printersSource
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
            signal highlight(string printer)
            width:  horizontalLayout ? parent.width * 0.60 - headerSeparator.width : parent.width
            height: horizontalLayout ? parent.height : parent.height - 50
            model: PlasmaCore.DataModel {
                id: jobsModel
                dataSource: jobsSource
            }
            delegate: JobItem{}
            
            Component.onCompleted: {
                printersView.highlight.connect(highlight);
            }
        }
    }
}
