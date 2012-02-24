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

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

PlasmaCore.DataModel {
    property alias interval: pmDataSource.interval
    property alias connectedSources: pmDataSource.connectedSources
    property string filter
    dataSource: PlasmaCore.DataSource {
        id: pmDataSource
        engine: "printmanager"
        onSourceAdded: {
            if (source.match("^" + filter)) {
//                 console.debug(filter + " ADDED: " + source);
                connectSource(source);
            }
        }
        onSourceRemoved: {
//             console.debug(filter + " REMOVED: " + source);
            disconnectSource(source);
        }
        Component.onCompleted: {
            for (var i in sources) {
                if (sources[i].match("^" + filter)) {
//                     console.debug(filter + "onCompleted ADDED: " + sources[i]);
                    connectSource(sources[i]);
                }
            }
        }
    }
}
