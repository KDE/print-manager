/*
 *   Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   Copyright 2014 Jan Grulich <jgrulich@redhat.com>
 *   Copyright 2021 Nate Graham <nate@kde.org>
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

    property bool cfg_allJobs
    property bool cfg_completedJobs
    property bool cfg_activeJobs

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
                return printerName === "" ? "" : i18nc("Printing with printer name", "Printing with %1", printerName);
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


    property var showAllJobsAction
    property var showCompletedJobsOnlyAction
    property var showActiveJobsOnlyAction

    function action_showAllJobs() {
        Plasmoid.configuration.allJobs = true;
        Plasmoid.configuration.completedJobs = false;
        Plasmoid.configuration.activeJobs = false;
    }

    function action_showCompletedJobsOnly() {
        Plasmoid.configuration.allJobs = false;
        Plasmoid.configuration.completedJobs = true;
        Plasmoid.configuration.activeJobs = false;
    }

    function action_showActiveJobsOnly() {
        Plasmoid.configuration.allJobs = false;
        Plasmoid.configuration.completedJobs = false;
        Plasmoid.configuration.activeJobs = true;
    }

    function action_configure() {
        KCMShell.openSystemSettings(printmanager.kcmName);
    }

    Component.onCompleted: {
        Plasmoid.setAction("showAllJobs", i18n("Show All Jobs"));
        printmanager.showAllJobsAction = Plasmoid.action("showAllJobs");
        printmanager.showAllJobsAction.checkable = true;
        printmanager.showAllJobsAction.checked = Qt.binding(() => {return Plasmoid.configuration.allJobs;});
        Plasmoid.setActionGroup("showAllJobs", "jobsShown");

        Plasmoid.setAction("showCompletedJobsOnly", i18n("Show Only Completed Jobs"));
        printmanager.showCompletedJobsOnlyAction = Plasmoid.action("showCompletedJobsOnly");
        printmanager.showCompletedJobsOnlyAction.checkable = true;
        printmanager.showCompletedJobsOnlyAction.checked = Qt.binding(() => {return Plasmoid.configuration.completedJobs;});
        Plasmoid.setActionGroup("showCompletedJobsOnly", "jobsShown");

        Plasmoid.setAction("showActiveJobsOnly", i18n("Show Only Active Jobs"));
        printmanager.showActiveJobsOnlyAction = Plasmoid.action("showActiveJobsOnly");
        printmanager.showActiveJobsOnlyAction.checkable = true;
        printmanager.showActiveJobsOnlyAction.checked = Qt.binding(() => {return Plasmoid.configuration.activeJobs;});
        Plasmoid.setActionGroup("showActiveJobsOnly", "jobsShown");

        // TODO: remove this separator once the configure action doesn't redundantly
        // appear in the hamburger menu
        Plasmoid.setActionSeparator("sep");

        Plasmoid.removeAction("configure");
        Plasmoid.setAction("configure", i18n("&Configure Printers..."), "configure");
        Plasmoid.action("configure").enabled = Qt.binding(() => {return kcmAllowed;});
    }
}
