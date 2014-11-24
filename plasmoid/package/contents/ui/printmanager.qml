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
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.printmanager 0.1 as PrintManager

Item {
    id: printmanager

    property int jobsFilter: printmanager.Plasmoid.configuration.allJobs ? PrintManager.JobModel.WhichAll :
                             printmanager.Plasmoid.configuration.completedJobs ? PrintManager.JobModel.WhichCompleted : PrintManager.JobModel.WhichActive

    Plasmoid.toolTipMainText: i18n("Printers");
    Plasmoid.icon: "printer";
    Plasmoid.compactRepresentation: CompactRepresentation { }
    Plasmoid.fullRepresentation: PopupDialog {
        id: dialogItem

        anchors.fill: parent
        focus: true
    }

    Plasmoid.switchWidth: units.gridUnit * 10
    Plasmoid.switchHeight: units.gridUnit * 10
    Plasmoid.status: (jobsFilterModel.activeCount > 0) ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus

    PrintManager.JobSortFilterModel {
        id: jobsFilterModel
        sourceModel: PrintManager.JobModel {
            id: jobsModel

            Component.onCompleted: {
                setWhichJobs(printmanager.jobsFilter)
            }
        }
    }
}
