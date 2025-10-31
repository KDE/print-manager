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
import org.kde.plasma.printmanager as PM

pragma ComponentBehavior: Bound

Kirigami.ScrollablePage {
    id: root

    property string currentFilterText: "Active Jobs"
    property bool expandedView: true

    required property KItemModels.KSortFilterProxyModel printersModel
    // provides special sort-by-jobState and filtered by printer
    required property PM.JobSortFilterModel jobsFilterModel
    // provides job filter (which jobs) and manage methods
    property PM.JobModel jobsModel: jobsFilterModel.sourceModel

    Connections {
        target: jobsModel

        function onLoaded() {
            jobs.positionViewAtBeginning()
        }
    }

    KItemModels.KDescendantsProxyModel {
        id: jobsProxy
        sourceModel: jobsFilterModel
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    actions: [
        Kirigami.Action {
            text: i18nc("@label:listbox", "Filter Jobs")
            icon.name: "filter-symbolic"
            visible: !root.searchMode

            Kirigami.Action {
              text: i18nc("option:check Show active Jobs", "Active Jobs")
              icon.name: "filter-symbolic"
              onTriggered: {
                  root.currentFilterText = text
                  jobsModel.jobFilter = PM.JobModel.WhichActive
              }
            }
            Kirigami.Action {
              text: i18nc("option:check Show completed jobs", "Completed Jobs")
              icon.name: "filter-symbolic"
              onTriggered: {
                  root.currentFilterText = text
                  jobsModel.jobFilter = PM.JobModel.WhichCompleted
              }
            }
            Kirigami.Action {
              text: i18nc("option:check Show all jobs", "All Jobs")
              icon.name: "filter-symbolic"
              onTriggered: {
                  root.currentFilterText = text
                  jobsModel.jobFilter = PM.JobModel.WhichAll
              }
            }
        },

        Kirigami.Action {
            text: root.expandedView
                  ? i18nc("option:check Hide expanded job view", "Hide Job Details")
                  : i18nc("option:check Show expanded job view", "Show Job Details")
            icon.name: "view-list-text-symbolic"
            checkable: true
            checked: root.expandedView
            displayHint: Kirigami.DisplayHint.IconOnly
            onTriggered: root.expandedView = !root.expandedView
        },

        Kirigami.Action {
            text: i18nc("@action:click", "About")
            icon.name: "help-about-symbolic"
            displayHint: Kirigami.DisplayHint.IconOnly
            onTriggered: applicationWindow().pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"), {}, {
                width: Kirigami.Units.gridUnit * 30,
            });
        }
    ]

    Kirigami.CardsListView {
        id: jobs
        clip: true
        spacing: Kirigami.Units.mediumSpacing

        model: jobsProxy

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.gridUnit * 4)
            icon.name: "data-information"
            text: i18nc("@info:status", "No %1", root.title)

            visible: jobs.count === 0
        }

        // see ipp_jstate_e in ipp.h: pending, hold, processing, stopped
        readonly property list<int> validMoveStates: [3,4,5,6]

        QQC2.Menu {
            id: moveMenu
            property var jobDel

            Repeater {
                model: printersModel

                delegate: QQC2.MenuItem {
                    required property string info
                    required property string printerName

                    text: i18n("Move to %1", info)
                    icon.name: "arrow-right-symbolic"
                    visible: moveMenu.jobDel?.jobPrinter !== printerName
                    onTriggered: {
                        jobsModel.move(moveMenu.jobDel.jobPrinter, moveMenu.jobDel.jobId, printerName)
                    }
                }
            }
        }

        component DragJobArea: MouseArea {
            anchors.fill: parent
            cursorShape: {
                if (jobs.validMoveStates.includes(jobState) && printersModel.count > 1) {
                    return pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                } else {
                    return undefined
                }
            }

            DragDrop.DragArea {
                anchors.fill: parent
                enabled: jobs.validMoveStates.includes(jobState) && printersModel.count > 1
                delegateImage: "document-new"
                mimeData.source: jobDelegate
            }
        }

        component CardLabel: Kirigami.Heading {
            level: 5
            elide: Text.ElideRight
            type: Kirigami.Heading.Type.Secondary
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing*2
        }

        delegate: ColumnLayout {
            id: jobDelegate
            width: ListView.view.width - Kirigami.Units.gridUnit*2

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
            required property string jobCompletedAt
            required property string jobProcessedAt
            required property string jobStateMsg
            required property bool jobCancelEnabled
            required property bool jobHoldEnabled
            required property bool jobReleaseEnabled
            required property bool jobRestartEnabled
            required property bool jobAuthRequired

            Kirigami.AbstractCard {
                headerOrientation: Qt.Horizontal
                Layout.fillWidth: true

                Kirigami.Theme.colorSet: Kirigami.Theme.Header
                Kirigami.Theme.inherit: false

                header: RowLayout {

                    Kirigami.Icon {
                        source: jobDelegate.jobIconName
                        DragJobArea {}
                    }

                    Kirigami.Heading {
                        text: jobDelegate.jobName
                        level: 4
                        type: Kirigami.Heading.Type.Primary
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        DragJobArea {}
                    }
                }

                footer: Kirigami.ActionToolBar {
                    actions: [
                        Kirigami.Action {
                            text: i18nc("@action:button", "Cancel")
                            icon.name: "dialog-cancel-symbolic"
                            visible: jobCancelEnabled
                            displayHint: Kirigami.DisplayHint.KeepVisible
                            onTriggered: {
                                enabled = false;
                                jobsModel.cancel(jobPrinter, jobId);
                                enabled = true;
                            }
                        },
                        Kirigami.Action {
                            text: i18nc("@action:button", "Hold")
                            icon.name: "media-playback-paused-symbolic"
                            visible: jobHoldEnabled
                            displayHint: Kirigami.DisplayHint.KeepVisible
                            onTriggered: {
                                enabled = false;
                                jobsModel.hold(jobPrinter, jobId);
                                enabled = true;
                            }
                        },
                        Kirigami.Action {
                            text: i18nc("@action:button", "Release")
                            icon.name: "media-playback-start-symbolic"
                            visible: jobReleaseEnabled
                            displayHint: Kirigami.DisplayHint.KeepVisible
                            onTriggered: {
                                enabled = false;
                                jobsModel.release(jobPrinter, jobId);
                                enabled = true;
                            }
                        },
                        Kirigami.Action {
                            text: i18nc("@action:button", "Reprint")
                            icon.name: "view-refresh-symbolic"
                            visible: jobRestartEnabled
                            onTriggered: {
                                enabled = false;
                                jobsModel.restart(jobPrinter, jobId);
                                enabled = true;
                            }
                        },
                        Kirigami.Action {
                            text: i18nc("@action:button", "Move")
                            icon.name: "transform-move-symbolic"
                            visible: jobs.validMoveStates.includes(jobState) && printersModel.count > 1
                            displayHint: Kirigami.DisplayHint.KeepVisible
                            onTriggered: {
                                jobs.currentIndex = index
                                moveMenu.jobDel = jobDelegate
                                moveMenu.popup()
                            }
                        },
                        Kirigami.Action {
                            text: i18nc("@action:button", "Authenticate")
                            icon.name: "view-refresh-symbolic"
                            visible: jobAuthRequired
                            onTriggered: {
                                enabled = false;
                                app.authenticateJob(jobPrinter, jobId, false);
                                enabled = true;
                            }
                        }
                    ]
                }

                contentItem: GridLayout {
                    columns: root.expandedView ? 2 : 0
                    visible: root.expandedView

                    CardLabel {
                        Layout.columnSpan: 2
                        text: {
                            let str = i18n("Status: %1", model.display)
                            if (jobStateMsg.length > 0) {
                                str += ", %1".arg(jobStateMsg)
                            }
                            return str
                        }
                    }

                    CardLabel {
                        opacity: jobCreatedAt.length > 0 ? .75 : 0
                        text: i18n("Created: %1", jobCreatedAt)
                    }
                    CardLabel {
                        Layout.alignment: Qt.AlignRight
                        Layout.fillWidth: false
                        text: jobSize
                    }

                    CardLabel {
                        Layout.columnSpan: 2
                        visible: jobCompletedAt.length > 0
                        text: i18n("Completed: %1", jobCompletedAt)
                    }

                    CardLabel {
                        visible: jobProcessedAt.length > 0
                        text: i18n("Processed: %1", jobProcessedAt)
                    }
                    CardLabel {
                        Layout.alignment: Qt.AlignRight
                        Layout.fillWidth: false
                        visible: jobPages.length > 0 && jobPages !== "0"
                        text: i18n("Pages: %1", jobPages)
                    }
                }

            }
        }

    }

}
