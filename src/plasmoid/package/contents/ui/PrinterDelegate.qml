/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
    SPDX-FileCopyrightText: 2024 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PrintManager
import org.kde.kitemmodels as KItemModels

PlasmaExtras.ExpandableListItem {
    id: delegate

    // Cannot use required property until ported away from "model".
    /*required */property PrintManager.JobModel printerJobsModel

    readonly property bool isPaused: model.printerState === 5

    icon: model.iconName
    iconEmblem: isPaused ? "emblem-pause" : ""
    title: model.info + (model.location && printersModel.displayLocationHint
            ? " (%1)".arg(model.location)
            : "")
    subtitle: model.stateMessage
    isDefault: model.isDefault

    customExpandedViewContent: jobsFilterModel.count > 0 ? jobListComponent : null

    defaultActionButtonAction: Kirigami.Action {
        icon.name: isPaused ? "media-playback-start" : "media-playback-pause"
        text: isPaused ? i18n("Resume") : i18n("Pause")

        onTriggered: {
            if (isPaused) {
                printersModel.resumePrinter(model.printerName);
            } else {
                printersModel.pausePrinter(model.printerName);
            }
        }
    }

    contextualActions: [
        Kirigami.Action {
            icon.name: "configure"
            text: i18n("Configure printer…")
            onTriggered: PrintManager.ProcessRunner.configurePrinter(model.printerName);
        },
        Kirigami.Action {
            icon.name: "view-list-details"
            text: i18n("View print queue…")
            onTriggered: PrintManager.ProcessRunner.openPrintQueue(model.printerName);
        }
    ]

    // Show max 3 jobs in the applet.
    KItemModels.KSortFilterProxyModel {
        id: jobsLimiterModel
        sourceModel: PrintManager.JobSortFilterModel {
            id: jobsFilterModel
            filteredPrinters: model.printerName
            sourceModel: delegate.printerJobsModel
        }

        filterRowCallback: (row, parent) => {
            return row < 3;
        }
        onRowsInserted: (parent, first, last) => {
            if (first < 3) {
                Qt.callLater(invalidateFilter);
            }
        }
        onRowsRemoved: (parent, first, last) => {
            if (last < 3) {
                Qt.callLater(invalidateFilter);
            }
        }
    }

    Component {
        id: jobListComponent

        ColumnLayout {
            spacing: 0

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.gridUnit
                Layout.rightMargin: Kirigami.Units.gridUnit
                spacing: 0

                Repeater {
                    id: jobRepeater

                    model: jobsLimiterModel

                    RowLayout {
                        id: jobDelegate

                        required property string jobId
                        required property string jobPrinter

                        required property string jobIconName
                        required property string jobName

                        required property int jobProcessedPagesCount
                        required property int jobPagesCount

                        required property bool jobCancelEnabled

                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            id: jobIcon
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                            source: jobDelegate.jobIconName
                        }

                        PlasmaComponents3.Label {
                            id: jobNameLabel
                            Layout.fillWidth: true
                            text: jobDelegate.jobName
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                        }

                        // Show page count when there's more than one page to print.
                        PlasmaComponents3.Label {
                            text: {
                                if (jobDelegate.jobProcessedPagesCount > 0) {
                                    if (jobDelegate.jobPagesCount > 0) {
                                        return i18ncp("Printing page %2 of %1", "%2/%1 page", "%2/%1 pages", jobDelegate.jobPagesCount, jobDelegate.jobProcessedPagesCount);
                                    } else {
                                        return i18nc("Printing page %1", "page %1", jobDelegate.jobProcessedPagesCount);
                                    }
                                } else if (jobDelegate.jobPagesCount > 0) {
                                    return i18np("%1 page", "%1 pages", jobDelegate.jobPagesCount);
                                } else {
                                    return "";
                                }
                            }
                            visible: text !== ""
                            enabled: false
                        }

                        PlasmaComponents3.ToolButton {
                            text: i18nc("@action:button", "Cancel")
                            icon.name: "dialog-cancel-symbolic"
                            visible: jobDelegate.jobCancelEnabled
                            onClicked: {
                                enabled = false;
                                jobsModel.cancel(jobDelegate.jobPrinter, jobDelegate.jobId);
                                enabled = true;
                            }
                        }
                    }
                }
            }

            RowLayout {
                id: moreJobsRow

                readonly property int count: jobsFilterModel.count - jobRepeater.count

                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.gridUnit// + Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.smallSpacing
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.minimumHeight: Kirigami.Units.gridUnit

                spacing: Kirigami.Units.smallSpacing
                visible: count > 0

                Kirigami.Icon {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    source: "view-more-symbolic"
                }

                PlasmaComponents3.Label {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    text: i18ncp("Number of additional print jobs", "%1 more job", "%1 more jobs", moreJobsRow.count)
                }
            }
        }
    }
}
