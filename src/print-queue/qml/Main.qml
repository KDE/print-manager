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
import org.kde.config // KAuthorized

pragma ComponentBehavior: Bound

Kirigami.ApplicationWindow {
    id: root

    property bool available: true
    onAvailableChanged: {
        if (pageStack.depth >= 1) {
            pageStack.pop()
        }
        if (!available) {
            pageStack.push(naComp)
        } else {
            pageStack.push(jobviewComp)
        }
    }

    Component.onCompleted: {
        available = Qt.binding(() => {return !printersModel.serverUnavailable && printers.count > 0})
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    color: Kirigami.Theme.backgroundColor

    Connections {
        target: app

        // Called with a queue name, select that queue and show jobs
        function onShowQueue(printerName: string): void {
            // Find the printer delegate that matches printer name
            // then set current
            let found = false
            for (let i = 0, len = printers.count; i < len; ++i) {
                const del = printers.itemAtIndex(i)
                if (del.printerName === printerName) {
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

    // Printers model
    KItemModels.KSortFilterProxyModel {
        id: printersProxy
        sourceModel: PM.PrinterModel {
            id: printersModel
            // emitted at startup (load) and anytime something changes a printer
            // or CUPS stops/starts
            onError: (err, title, msg) => {
                // err === 0 means successful load
                if (err !== 0 && !available) {
                    if (pageStack.depth >= 1) {
                        pageStack.pop()
                    }
                    pageStack.push(naComp)
                }
            }
        }
        sortRoleName: "isClass"
    }

    // Jobs model
    PM.JobSortFilterModel {
        id: jobsSortFilterModel
        sortRole: PM.JobModel.RoleJobState

        sourceModel: PM.JobModel {
            id: jobsModel

            onError: (err, title, msg) => {
                // Force a reload in case CUPS services are unavailable
                console.warn(err, title, msg)
                printersModel.update()
            }

            onMessagesChanged: {
                if (messages.length > 0) {
                  bottomDrawer.open()
                } else {
                  bottomDrawer.close()
                }
            }
        }
    }

    // Printers View
    globalDrawer: Kirigami.GlobalDrawer {
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        edge: Qt.application.layoutDirection === Qt.RightToLeft ? Qt.RightEdge : Qt.LeftEdge
        modal: printers.count === 1
        width: Math.round(root.width * .25)

        header: QQC2.ToolBar {
            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Item {
                    implicitWidth: Kirigami.Units.largeSpacing
                }

                Kirigami.Heading {
                    text: i18nc("@label:header", "Printers")
                    Layout.fillWidth: true
                }

                QQC2.ToolButton {
                    id: aboutButton
                    text: i18nc("@action:button about this application", "About")
                    icon.name: "help-about-symbolic"
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    display: QQC2.ToolButton.IconOnly

                    onClicked: applicationWindow().pageStack.pushDialogLayer(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"), {}, {
                        width: Kirigami.Units.gridUnit * 30,
                    });

                    QQC2.ToolTip.text: aboutButton.text
                    QQC2.ToolTip.visible: hovered || activeFocus
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }

        ListView {
            id: printers
            clip: true
            spacing: Kirigami.Units.smallSpacing

            Layout.fillWidth: true
            Layout.fillHeight: true

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            Component {
                id: sectionComp

                Kirigami.ListSectionHeader {
                    width: ListView.view.width
                    required property bool section
                    text: !section ? i18nc("@label:info device description", "Printers") : i18nc("@label:info device description", "Printer Groups")
                }
            }

            // If there is a mix of printers and classes (groups), then show
            // the section header
            section {
                property: "isClass"
                delegate: !printersModel.hasOnlyPrinters ? sectionComp : undefined
            }

            model: printersProxy

            onCurrentIndexChanged: jobsSortFilterModel.filteredPrinters = currentItem.printerName

            delegate: QQC2.ItemDelegate {
                id: deviceDelegate
                implicitWidth: ListView.view.width

                required property int index
                required property bool isClass
                required property int printerState
                required property bool isPaused
                required property bool isDefault
                required property bool remote
                required property string printerName
                required property string location
                required property string info
                required property string stateMessage
                required property string iconName

                // 3: Idle, 4: Printing, 5: Stopped (paused)
                onPrinterStateChanged: {
                    if (printerState === 4) {
                        jobsModel.jobFilter = PM.JobModel.WhichActive
                    }
                }

                text: info
                      + (location && printersModel.showLocations
                         ? " (%1)".arg(location)
                         : "")
                Accessible.description: titleItem.subtitle

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
                        }
                        containsAcceptableDrag = false;
                    }
                }

                highlighted: ListView.isCurrentItem
                onClicked: ListView.view.currentIndex = index

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.IconTitleSubtitle {
                        id: titleItem
                        Layout.preferredWidth: deviceDelegate.width * .75
                        title: deviceDelegate.text
                        subtitle: stateMessage
                        icon: icon.fromControlsIcon(deviceDelegate.icon)
                        font.bold: printers.count > 1 & isDefault
                    }

                    Kirigami.ActionToolBar {
                        spacing: 0
                        display: QQC2.Button.IconOnly

                        Layout.fillWidth: true
                        alignment: Qt.AlignRight

                        actions: [
                            Kirigami.Action {
                                icon.name: isPaused
                                           ? "media-playback-start-symbolic"
                                           : "media-playback-pause-symbolic"
                                text: isPaused
                                      ? i18nc("@action:button Resume printing", "Resume")
                                      : i18nc("@action:button Pause printing", "Pause")

                                displayHint: Kirigami.DisplayHint.KeepVisible

                                onTriggered: {
                                    if (isPaused) {
                                        printersModel.resumePrinter(printerName);
                                    } else {
                                        printersModel.pausePrinter(printerName);
                                    }
                                }
                            },
                            Kirigami.Action {
                                text: i18nc("@action:button Configure printer in the kcm", "Configure…")
                                icon.name: "settings-configure-symbolic"
                                enabled: KAuthorized.authorizeControlModule("kcm_printer_manager")

                                onTriggered: PM.ProcessRunner.kcmConfigurePrinter(printerName)
                            },
                            Kirigami.Action {
                                text: i18nc("@action:button Open additional printer settings", "Media Settings…")
                                icon.name: "paper-color-symbolic"

                                onTriggered: PM.ProcessRunner.configurePrinter(printerName)
                            }
                        ]
                    }
                }
            }
        }
    }

    // Job Processing messages
    Kirigami.OverlayDrawer {
        id: bottomDrawer
        edge: Qt.BottomEdge
        modal: true

        contentItem: QQC2.ScrollView {
            width: bottomDrawer.width
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
            clip: true

            ListView {
                width: parent.width
                spacing: 0
                clip: true
                model: jobsModel.messages

                delegate: Kirigami.IconTitleSubtitle {
                    required property var modelData
                    title: modelData
                    reserveSpaceForSubtitle: false
                    icon.name: "task-process-1-symbolic"
                }
            }
        }
    }

    // Jobs
    Component {
        id: jobviewComp

        JobView {
            title: "%1 [%2]".arg(currentFilterText).arg(printers.currentItem?.info)

            printersModel: printersProxy
            jobsFilterModel: jobsSortFilterModel
        }
    }

    // Feedback placeholders
    Component {
        id: naComp

        NotAvailablePage {
            deviceCount: printers.count
            serverAvailable: !printersModel.serverUnavailable
        }
    }

}
