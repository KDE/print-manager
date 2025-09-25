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

    property string currentFilterText
    property bool expandedView: true

    required property KItemModels.KSortFilterProxyModel printersModel
    // provides special sort-by-jobState and filtered by printer
    required property PM.JobSortFilterModel jobsFilterModel
    // provides job filter (which jobs) and manage methods
    property PM.JobModel jobsModel: jobsFilterModel.sourceModel

    function setCurrentFilterText(text: string) : void {
        currentFilterText = i18nc("@label Job types currently showing", "%1 Jobs", text)
    }

    Component.onCompleted: setCurrentFilterText(activeAction.text)

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

            Kirigami.Action {
                id: activeAction
                text: i18nc("option:check Show active Jobs", "Active")
                icon.name: "filter-symbolic"
                onTriggered: {
                    root.setCurrentFilterText(text)
                    jobsModel.jobFilter = PM.JobModel.WhichActive
                }
            }
            Kirigami.Action {
                text: i18nc("option:check Show completed jobs", "Completed")
                icon.name: "filter-symbolic"
                onTriggered: {
                    root.setCurrentFilterText(text)
                    jobsModel.jobFilter = PM.JobModel.WhichCompleted
                }
            }
            Kirigami.Action {
                text: i18nc("option:check Show all jobs", "All")
                icon.name: "filter-symbolic"
                onTriggered: {
                    root.setCurrentFilterText(text)
                    jobsModel.jobFilter = PM.JobModel.WhichAll
                }
            }
        },

        Kirigami.Action {
            text: root.expandedView
                  ? i18nc("@option:check Hide expanded job view", "Hide Job Details")
                  : i18nc("@option:check Show expanded job view", "Show Job Details")
            icon.name: root.expandedView ? "view-list-details-symbolic" : "view-list-text-symbolic"
            displayHint: Kirigami.DisplayHint.IconOnly
            onTriggered: root.expandedView = !root.expandedView
        },

        Kirigami.Action {
            text: i18nc("@action:button Cancel all active jobs", "Cancel All Jobs")
            icon.name: "dialog-cancel-symbolic"
            visible: jobsFilterModel.count > 0 && jobsModel.jobFilter === PM.JobModel.WhichActive
            displayHint: Kirigami.DisplayHint.IconOnly
            onTriggered: cancelPrompt.active = true
        }
    ]

    Loader {
        id: cancelPrompt
        active: false

        sourceComponent: Kirigami.PromptDialog {
            id: prompt
            title: i18nc("@title:window", "Cancel all active print jobs?")

            parent: root
            standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
            dialogType: Kirigami.PromptDialog.Warning

            Component.onCompleted: open()
            onClosed: cancelPrompt.active = false

            onAccepted: {
                jobsModel.cancelAll(jobsFilterModel.filteredPrinters)
                prompt.close()
            }
        }
    }

    Kirigami.CardsListView {
        id: jobs
        clip: true
        spacing: Kirigami.Units.mediumSpacing

        model: jobsProxy

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.gridUnit * 4)
            icon.name: "data-information"
            text: i18nc("@info:status Print jobs not found in the print queue", "The print queue is empty")

            visible: jobs.count === 0
        }

        // see ipp_jstate_e in ipp.h: pending, hold, processing, stopped
        readonly property list<int> validMoveStates: [3, 4, 5, 6]

        QQC2.Menu {
            id: moveMenu
            property var jobDel

            Repeater {
                model: printersModel

                delegate: QQC2.MenuItem {
                    required property string info
                    required property string printerName

                    text: i18nc("@action:inmenu Move the job to /info/ which is the friendly name of the other print queue", "Move to %1", info)
                    icon.name: "printer-symbolic"
                    visible: moveMenu.jobDel?.jobPrinter !== printerName
                    onTriggered: {
                        jobsModel.move(moveMenu.jobDel.jobPrinter, moveMenu.jobDel.jobId, printerName)
                    }
                }
            }
        }

        component DragJobArea: MouseArea {
            id: mouseArea
            anchors.fill: parent
            cursorShape: {
                if (dragArea.enabled) {
                    return mouseArea.containsPress ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                } else {
                    return Qt.ArrowCursor
                }
            }

            DragDrop.DragArea {
                id: dragArea
                anchors.fill: parent
                enabled: jobs.validMoveStates.includes(jobState) && printersModel.count > 1
                delegateImage: "edit-move"
                mimeData.source: jobDelegate
            }
        }

        component CardLabel: Kirigami.Heading {
            level: 4
            elide: Text.ElideRight
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 2
        }

        delegate: ColumnLayout {
            id: jobDelegate
            width: ListView.view.width - Kirigami.Units.gridUnit * 2

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
                    spacing: Kirigami.Units.smallSpacing

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
                            text: i18nc("@action:button This action cancels the job", "Cancel")
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
                            text: i18nc("@action:button This action holds the job", "Hold")
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
                            text: i18nc("@action:button This action releases the job", "Release")
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
                            text: i18nc("@action:button This action reprints the job", "Reprint")
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
                            text: i18nc("@action:button This action prompts for user/password to authenticate the job", "Authenticate")
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
                    rowSpacing: Kirigami.Units.smallSpacing
                    columnSpacing: Kirigami.Units.smallSpacing
                    columns: root.expandedView ? 2 : 0
                    visible: root.expandedView

                    CardLabel {
                        Layout.columnSpan: 2
                        text: jobStateMsg.length > 0
                              ? i18nc("@label:info", "Status: %1, %2", model.display, jobStateMsg)
                              : i18nc("@label:info", "Status: %1", model.display)
                    }

                    CardLabel {
                        opacity: jobCreatedAt.length > 0 ? .75 : 0
                        text: i18nc("@label:info Date created", "Created: %1", jobCreatedAt)
                    }
                    CardLabel {
                        Layout.alignment: Qt.AlignRight
                        Layout.fillWidth: false
                        text: jobSize
                    }

                    CardLabel {
                        Layout.columnSpan: 2
                        visible: jobCompletedAt.length > 0
                        text: i18nc("@label:info Date completed", "Completed: %1", jobCompletedAt)
                    }

                    CardLabel {
                        visible: jobProcessedAt.length > 0
                        text: i18nc("@label:info Date processed", "Processed: %1", jobProcessedAt)
                    }
                    CardLabel {
                        Layout.alignment: Qt.AlignRight
                        Layout.fillWidth: false
                        visible: jobPages.length > 0 && jobPages !== "0"
                        text: i18nc("@label:info Job page count", "Pages: %1", jobPages)
                    }
                }

            }
        }

    }

}
