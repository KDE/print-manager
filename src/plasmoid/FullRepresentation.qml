/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.printmanager as PrintManager

PlasmaExtras.Representation {
    id: fullRep
    required property PrintManager.JobModel printerJobsModel
    required property bool serverUnavailable
    required property PrintManager.PrinterModel printersModel
    required property string printersModelError

    collapseMarginsHint: true

    header: PlasmaExtras.PlasmoidHeading {
        PlasmaExtras.SearchField {
            anchors.fill: parent
            onTextChanged: printersFilterModel.filterString = text.toLowerCase()
        }
    }

    PlasmaComponents3.ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth - printerList.leftMargin - printerList.rightMargin
        PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

        contentItem: ListView {
            id: printerList
            focus: true
            currentIndex: -1

            section {
                property: fullRep.printersModel.hasOnlyPrinters ? "" : "isClass"
                delegate: Kirigami.ListSectionHeader {
                    width: ListView.view.width
                    required property bool section
                    text: !section ? i18n("Printers") : i18n("Printer Groups")
                }
            }
            
            model: KItemModels.KSortFilterProxyModel {
                id: printersFilterModel
                sourceModel: fullRep.printersModel
                sortRoleName: "isClass"
                
                filterRowCallback: (source_row, source_parent) => {
                    return sourceModel.data(sourceModel.index(source_row, 0, source_parent), PrintManager.PrinterModel.DestDescription).toLowerCase().includes(filterString)
                }

            }

            topMargin: Kirigami.Units.smallSpacing * 2
            bottomMargin: Kirigami.Units.smallSpacing * 2
            leftMargin: Kirigami.Units.smallSpacing * 2
            rightMargin: Kirigami.Units.smallSpacing * 2
            spacing: Kirigami.Units.smallSpacing

            highlight: PlasmaExtras.Highlight {}
            highlightMoveDuration: Kirigami.Units.shortDuration
            highlightResizeDuration: Kirigami.Units.shortDuration
            delegate: PrinterDelegate {
                printerJobsModel: fullRep.printerJobsModel
                printersModel: fullRep.printersModel
            }

            Loader {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                active: printersFilterModel.count === 0 || fullRep.serverUnavailable
                sourceComponent: PlasmaExtras.PlaceholderMessage {
                    text: fullRep.serverUnavailable ? fullRep.printersModelError || i18n("No printers have been configured or discovered") : i18n("No matches")
                    iconName: fullRep.serverUnavailable ? "dialog-error" : "edit-none"
                }
            }
        }
    }
}
