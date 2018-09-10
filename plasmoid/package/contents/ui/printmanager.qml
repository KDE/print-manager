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
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.printmanager 0.2 as PrintManager

Item {
    id: printmanager

    property int jobsFilter: printmanager.Plasmoid.configuration.allJobs ? PrintManager.JobModel.WhichAll :
                             printmanager.Plasmoid.configuration.completedJobs ? PrintManager.JobModel.WhichCompleted : PrintManager.JobModel.WhichActive

    property alias serverUnavailable: printersModel.serverUnavailable
    property string printersModelError: ""

    readonly property string kcmName: "kcm_printer_manager"
    readonly property bool kcmAllowed: KCMShell.authorize(kcmName + ".desktop").length > 0

    Plasmoid.toolTipMainText: i18n("Printers")
    Plasmoid.toolTipSubText: {
        if (serverUnavailable && printersModelError) {
            return printersModelError;
        } else if (activeJobsFilterModel.activeCount > 1) {
            return i18np("There is one print job in the queue",
                         "There are %1 print jobs in the queue",
                         activeJobsFilterModel.activeCount);
        // If there is only one job, show more information about it
        } else if (activeJobsFilterModel.activeCount === 1) {
            var idx = activeJobsFilterModel.index(0, 0);
            var jobName = activeJobsFilterModel.data(idx, PrintManager.JobModel.RoleJobName);
            var printerName = activeJobsFilterModel.data(idx, PrintManager.JobModel.RoleJobPrinter);
            if (jobName) {
                return i18nc("Printing document name with printer name", "Printing %1 with %2", jobName, printerName);
            } else {
                return i18nc("Printing with printer name", "Printing with %1", printerName);
            }
        } else if (printersModel.count > 0) {
            return i18n("Print queue is empty");
        } else {
            return i18n("No printers have been configured or discovered");
        }
    }
    Plasmoid.icon: "printer"
    Plasmoid.fullRepresentation: PopupDialog {
        id: dialogItem

        anchors.fill: parent
        focus: true
    }

    Plasmoid.switchWidth: units.gridUnit * 10
    Plasmoid.switchHeight: units.gridUnit * 10
    Plasmoid.status: {
        if (activeJobsFilterModel.activeCount > 0) {
            return PlasmaCore.Types.ActiveStatus;
        } else if (printersModel.count > 0 || serverUnavailable) {
            return PlasmaCore.Types.PassiveStatus;
        } else {
            return PlasmaCore.Types.HiddenStatus;
        }
    }

    onJobsFilterChanged: jobsModel.setWhichJobs(jobsFilter)
    Component.onCompleted: {
        if (kcmAllowed) {
            plasmoid.setAction("printerskcm", i18n("&Configure Printers..."), "printer");
        }
    }

    PrintManager.PrinterModel {
        id: printersModel
        onError: printersModelError = errorTitle
    }

    PrintManager.JobSortFilterModel {
        id: jobsFilterModel

        sourceModel: PrintManager.JobModel {
            id: jobsModel
            Component.onCompleted: setWhichJobs(printmanager.jobsFilter)
        }
    }

    PrintManager.JobSortFilterModel {
        id: activeJobsFilterModel
        sourceModel: PrintManager.JobModel {
            Component.onCompleted: setWhichJobs(PrintManager.JobModel.WhichActive)
        }
    }

    function action_printerskcm() {
        KCMShell.open([printmanager.kcmName]);
    }
}
