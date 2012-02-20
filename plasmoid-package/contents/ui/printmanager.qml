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
    property int minimumWidth: 400
    property int minimumHeight: 300

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.DataSource {
        id: printersSource
        engine: "printmanager"
        connectedSources: ["Printers"]
        interval: 500
        
        onSourceAdded: {
            console.debug(source);
            var pattern = /Printers\/[^\/]+$/
            if (source.match(pattern)) {
                console.debug("Source Connected:" + source);
                connectSource(source);
            }            
        }
        Component.onCompleted: {
            var pattern = /Printers\/[^\/]+$/
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
        connectedSources: ["ActiveJobs"]
        interval: 500
        property string whichJobs: "ActiveJobs"
        property string whichPrinter
        
        onSourceAdded: {
            console.debug(source);
            var pattern = /ActiveJobs\/\d+$/
            if (source.match(pattern)) {
                console.debug("Source Connected:" + source);
                connectSource(source);
            }            
        }
        Component.onCompleted: {
            var pattern = /ActiveJobs\/\d+$/
            for (var i in sources) {
                console.debug("COMPLETED " + sources[i]);
                if (sources[i].match(pattern)) {
                    console.debug("COMPLETED Source Connected:" + sources[i]);
                    connectSource(sources[i]);
                }
            }
        }
    }
    
    
//     Row {
//         spacing: 2
//         anchors.fill: parent
        ScrollableListView {
            id: printersView
            anchors {
                left: parent.left
                right: jobsView.left
                top: parent.top
                bottom: parent.bottom
            }
            model: PlasmaCore.DataModel {
                id: printersModel
                dataSource: printersSource
            }
            delegate: PrinterItem{}
        }
        
        ScrollableListView {
            id: jobsView
            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            model: PlasmaCore.DataModel {
                id: jobsModel
                dataSource: printersSource
            }
            delegate: PrinterItem{}
        }
//     }
}
