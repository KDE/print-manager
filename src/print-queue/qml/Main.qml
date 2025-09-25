/*
 *   SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.draganddrop as DragDrop
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.printmanager as PM

pragma ComponentBehavior: Bound

Kirigami.ApplicationWindow {
    id: root

    readonly property bool wideMode: root.width >= Kirigami.Units.gridUnit * 50

    Connections {
        target: app

        // find the printer delegate that matches dest name
        // then set current
        function onShowQueue(dest) {
            let found = false
            for (let i=0, len=printers.count; i<len; ++i) {
                const del = printers.itemAtIndex(i)
                if (del.printerName === dest) {
                    printers.currentIndex = i
                    found = true
                    break
                }
            }
            if (!found) {
                printers.currentIndex = 0
            }
        }
    }

    globalDrawer: Kirigami.OverlayDrawer {
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        edge: Qt.application.layoutDirection === Qt.RightToLeft ? Qt.RightEdge : Qt.LeftEdge
        modal: !root.wideMode
        handleVisible: modal
        onModalChanged: drawerOpen = !modal
        width: Kirigami.Units.gridUnit * 16

        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false

        contentItem: ColumnLayout {
            spacing: 0

            QQC2.ToolBar {
                Layout.fillWidth: true
                height: root.pageStack.globalToolBar.preferredHeight

                topPadding: Kirigami.Units.smallSpacing / 2

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.ComboBox {
                        id: filterSelector
                        enabled: printers.count > 0

                        Layout.fillWidth: true

                        model: [{ value: PM.JobModel.WhichActive, text: i18n("Active Jobs") },
                            { value: PM.JobModel.WhichCompleted, text: i18n("Completed Jobs") },
                            { value: PM.JobModel.WhichAll, text: i18n("All Jobs") }]

                        textRole: "text"
                        valueRole: "value"

                        Component.onCompleted: {
                            currentIndex = 0
                            jobsModel.setWhichJobs(filterSelector.currentValue)
                        }

                        onActivated: {
                            jobsModel.setWhichJobs(filterSelector.currentValue)
                        }

                    }

                    QQC2.ToolButton {
                        text: i18nc("@action:click", "About")
                        icon.name: "documentinfo-symbolic"
                        display: QQC2.ToolButton.IconOnly
                        onClicked: root.pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"), {}, {
                            width: Kirigami.Units.gridUnit * 30,
                        });
                    }
                }
            }

            // Printers
            QQC2.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false

                ListView {
                    id: printers
                    clip: true
                    spacing: Kirigami.Units.smallSpacing

                    highlight: PlasmaExtras.Highlight {}
                    highlightMoveDuration: Kirigami.Units.shortDuration
                    highlightResizeDuration: Kirigami.Units.shortDuration

                    Component {
                        id: sectionComp

                        Kirigami.ListSectionHeader {
                            width: ListView.view.width
                            required property bool section
                            text: !section ? i18n("Printers") : i18n("Printer Groups")
                        }
                    }

                    // If there is a mix of printers and classes (groups), then show
                    // the section header
                    section {
                        property: "isClass"
                        delegate: !pmModel.hasOnlyPrinters ? sectionComp : undefined
                    }

                    model: KItemModels.KSortFilterProxyModel {
                        sourceModel: PM.PrinterModel {
                            id: pmModel
                        }
                        sortRoleName: "isClass"
                    }

                    onCurrentIndexChanged: jobsSortFilterModel.filteredPrinters = currentItem.printerName

                    QQC2.Menu {
                        id: menu
                        property QQC2.ItemDelegate devDelegate

                        Kirigami.Action {
                            icon.name: menu.devDelegate.isPaused
                                       ? "media-playback-start-symbolic"
                                       : "media-playback-pause-symbolic"
                            text: menu.devDelegate.isPaused
                                  ? i18nc("@action:button Resume printing", "Resume")
                                  : i18nc("@action:button Pause printing", "Pause")

                            onTriggered: {
                                if (menu.devDelegate.isPaused) {
                                    pmModel.resumePrinter(menu.devDelegate.printerName);
                                } else {
                                    pmModel.pausePrinter(menu.devDelegate.printerName);
                                }
                            }
                        }

                        Kirigami.Action {
                            text: i18nc("@action:button Open additional printer settings", "Media Settings…")
                            icon.name: "settings-configure-symbolic"

                            onTriggered: PM.ProcessRunner.configurePrinter(menu.devDelegate.printerName)
                        }
                    }

                    delegate: QQC2.ItemDelegate {
                        id: deviceDelegate
                        implicitWidth: ListView.view.width

                        required property var model
                        required property int index
                        required property bool isClass
                        required property bool isPaused
                        required property bool isDefault
                        required property bool remote
                        required property string printerName
                        required property string location
                        required property string info
                        required property string stateMessage
                        required property string iconName

                        text: info
                              + (location && pmModel.showLocations
                                 ? " (%1)".arg(location)
                                 : "")
                        icon.name: remote
                                ? "folder-network-symbolic"
                                : (isClass ? "folder-print" : iconName)
                        icon.width: Kirigami.Units.iconSizes.medium
                        icon.height: Kirigami.Units.iconSizes.medium

                        DragDrop.DropArea {
                            anchors.fill: parent

                            property bool containsAcceptableDrag: false

                            onDragEnter: event => {
                                if (!event.mimeData.source) {
                                    event.ignore()
                                    return
                                }

                                containsAcceptableDrag = event.mimeData.source.jobPrinter !== printerName
                                if (!containsAcceptableDrag) {
                                    event.ignore();
                                }
                            }

                            onDragLeave: event => {
                                containsAcceptableDrag = false
                            }

                            onDrop: event => {
                                if (containsAcceptableDrag) {
                                    jobsModel.move(event.mimeData.source.jobPrinter, event.mimeData.source.jobId, printerName)
                                    jobsModel.setWhichJobs(filterSelector.currentValue)
                                }
                                containsAcceptableDrag = false;
                            }
                        }

                        highlighted: ListView.isCurrentItem

                        onClicked: ListView.view.currentIndex = index

                        contentItem: RowLayout {
                            spacing: Kirigami.Units.smallSpacing

                            Kirigami.IconTitleSubtitle {
                                Layout.fillWidth: true
                                title: deviceDelegate.text
                                subtitle: stateMessage
                                icon: icon.fromControlsIcon(deviceDelegate.icon)
                                font.bold: printers.count > 1 & isDefault
                            }

                            QQC2.ToolButton {
                                visible: deviceDelegate.hovered
                                icon.name: "overflow-menu-symbolic"
                                onClicked: {
                                    printers.currentIndex = index
                                    menu.devDelegate = deviceDelegate
                                    menu.popup()
                                }

                            }

                        }
                    }
                }
            }
        }
    }

    pageStack.initialPage: Kirigami.ScrollablePage {
        title: "%1 for %2".arg(filterSelector.currentText).arg(printers.currentItem.info)

        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false

        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        // Jobs
        ListView {
            id: jobs
            clip: true

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            model: KItemModels.KDescendantsProxyModel {
                sourceModel: PM.JobSortFilterModel {
                    id: jobsSortFilterModel
                    sourceModel: PM.JobModel {
                        id: jobsModel
                    }
                    sortRole: PM.JobModel.RoleJobState
                }
            }

            Component {
                id: noJobsMsgComp

                Kirigami.PlaceholderMessage {
                    icon.name: "edit-none"
                    text: i18nc("@info:status", "%1 not found", filterSelector.currentText)
                }
            }

            Component {
                id: noDevicesMsgComp

                Kirigami.PlaceholderMessage {
                    icon.name: "printer-warning"
                    text: i18nc("@info:status", "No printers are currently configured")
                    helpfulAction: Kirigami.Action {
                        text: i18nc("@action:button", "Configure…")
                        icon.name: "printer-symbolic"
                        onTriggered: PM.ProcessRunner.openKCM()
                    }
                }
            }

            Component {
                id: noServiceMsgComp

                Kirigami.PlaceholderMessage {
                    icon.name: "printer-error"
                    text: i18nc("@info:status", "Printing services not available")
                    explanation: xi18nc("@info:usagetip", "Verify that the CUPS system service is active.")
                    helpfulAction: Kirigami.Action {
                        text: i18nc("@action:button", "Refresh")
                        icon.name: "view-refresh"
                        onTriggered: pmModel.update()
                    }
                }
            }

            Loader {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.gridUnit * 4)

                active: pmModel.serverUnavailable || printers.count === 0 || jobs.count === 0

                sourceComponent: {
                    let ret = null
                    if (pmModel.serverUnavailable) {
                        ret = noServiceMsgComp
                    } else if (printers.count === 0) {
                        ret = noDevicesMsgComp
                    } else if (jobs.count === 0) {
                        ret = noJobsMsgComp
                    }

                    return ret
                }
            }

            // see ipp_jstate_e in ipp.h, pending, hold, processing, stopped
            readonly property list<int> validMoveStates: [3,4,5,6]

            delegate: QQC2.ItemDelegate {
                id: jobDelegate
                implicitWidth: ListView.view.width

                required property var model
                required property int index
                required property int jobState
                required property int jobId
                required property string jobPrinter
                required property string jobName
                required property string jobIconName
                required property string jobSize
                required property string jobPages
                required property string jobCreatedAt
                required property string jobStateMsg
                required property bool jobCancelEnabled
                required property bool jobHoldEnabled
                required property bool jobReleaseEnabled
                required property bool jobRestartEnabled
                required property bool jobAuthRequired

                text: {
                    let str = "%1: %2".arg(jobName).arg(jobSize)
                    if (jobPages.length > 0 && jobPages !== "0") {
                        str += i18n(", Pages: %1", jobPages)
                    }
                    return str
                }
                icon.name: jobDelegate.jobIconName
                icon.width: Kirigami.Units.iconSizes.smallMedium
                icon.height: Kirigami.Units.iconSizes.smallMedium

                hoverEnabled: true

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.IconTitleSubtitle {
                        Layout.fillWidth: true

                        title: jobDelegate.text
                        subtitle: {
                            let str = model.display
                            if (jobCreatedAt.length > 0) {
                                str += i18n(", Created: %1", jobCreatedAt)
                            }
                            if (jobStateMsg.length > 0) {
                                str += ", %1".arg(jobStateMsg)
                            }
                            return str
                        }
                        icon: icon.fromControlsIcon(jobDelegate.icon)

                        DragDrop.DragArea {
                            anchors.fill: parent
                            enabled: jobs.validMoveStates.includes(jobState)
                            delegateImage: "document-new"
                            mimeData.source: jobDelegate
                        }
                    }

                    // floating actions
                    RowLayout {
                        visible: jobDelegate.hovered
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.ToolButton {
                            text: i18nc("@action:button", "Cancel")
                            icon.name: "dialog-cancel-symbolic"
                            visible: jobCancelEnabled
                            onClicked: {
                                enabled = false;
                                jobsModel.cancel(jobPrinter, jobId);
                                enabled = true;
                            }
                        }
                        QQC2.ToolButton {
                            text: i18nc("@action:button", "Hold")
                            icon.name: "media-playback-paused-symbolic"
                            visible: jobHoldEnabled
                            onClicked: {
                                enabled = false;
                                jobsModel.hold(jobPrinter, jobId);
                                enabled = true;
                            }
                        }

                        QQC2.ToolButton {
                            text: i18nc("@action:button", "Release")
                            icon.name: "media-playback-start-symbolic"
                            visible: jobReleaseEnabled
                            onClicked: {
                                enabled = false;
                                jobsModel.release(jobPrinter, jobId);
                                enabled = true;
                            }
                        }
                        QQC2.ToolButton {
                            text: i18nc("@action:button", "Reprint")
                            icon.name: "view-refresh-symbolic"
                            visible: jobRestartEnabled
                            onClicked: {
                                enabled = false;
                                jobsModel.restart(jobPrinter, jobId);
                                enabled = true;
                            }
                        }

                        QQC2.ToolButton {
                            text: i18nc("@action:button", "Authenticate")
                            icon.name: "view-refresh-symbolic"
                            visible: jobAuthRequired
                            onClicked: {
                                enabled = false;
                                app.authenticateJob(jobPrinter, jobId, false);
                                enabled = true;
                            }
                        }

                    }
                }
            }

        }

    }
}
