/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KSFM
import org.kde.plasma.components as PComp
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.printmanager as PM

/*
* PPD driver selection
*/
Kirigami.Dialog {
    id: root

    property PM.PPDModel model
    property bool loading: true
    // ppd attributes for the make/model
    property var ppdData: ({})

    // make is the first level in the hierarchy of the model
    function getMakeIndex(make) {
        print("GETMAKE:", make)
        for (let i=0, len=model.rowCount(); i<len; ++i) {
            const val = model.data(model.index(i,0), Qt.DisplayRole).toString()
            if (val === make)
                return i
        }
        return -1
    }

    // make/model is the second level
    // try to select based on CUPS pcfile *and* makemodel desc
    // preference to pcfile
    function getMakeModelIndex() {
        const file = ppdData.pcfile?.toLowerCase()
        const mm = ppdData.makeModel?.toLowerCase()
        print("GETMAKEMODEL pcfile:", file, ", mm:", mm)
        for (let i=0, len=printerModels.rowCount(); i<len; ++i) {
            const ndx = printerModels.mapToSource(printerModels.index(i,0))
            const f = printerModels.sourceModel.data(ndx, PM.PPDModel.PPDName).toString()
            const m = printerModels.sourceModel.data(ndx, PM.PPDModel.PPDMakeAndModel).toString()
            if ((file && f && f.toLowerCase().includes(file)) | (mm && m && m.toLowerCase().includes(mm))) {
                return i
            }
        }
        return -1
    }

    function setCurrentMakeModel(index = 0) {
        if (index === -1) {
            return
        }
        const ndx = printerModels.mapToSource(printerModels.index(index,0))
        ppdData.makeModel = printerModels.sourceModel.data(ndx, PM.PPDModel.PPDMakeAndModel).toString()
        ppdData.file = printerModels.sourceModel.data(ndx, PM.PPDModel.PPDName).toString()
    }

    function init() {
        if (ppdData.type === PM.PPDType.Manual) {
            rbFile.checked = true
            customFilename.text = ppdData.file
        } else {
            rbMake.checked = true

            // make/model list selections
            makesList.currentIndex = getMakeIndex(ppdData.make)
            makesList.positionViewAtIndex(makesList.currentIndex, ListView.Center)
            printerModels.invalidateFilter()
            // select the makeModel
            Qt.callLater(() => {
                makeModelList.currentIndex = getMakeModelIndex()
                makeModelList.positionViewAtIndex(makeModelList.currentIndex, ListView.Center)
                setCurrentMakeModel(makeModelList.currentIndex)
            })
        }

        title = i18nc("@title:window", "Printer Driver (%1)", ppdData.makeModel)
    }

    signal saveValues(var data)

    title: i18nc("@title:window", "Printer Driver")

    onLoadingChanged: {
        print("PPD Database Count:", model.rowCount())
        if (!loading) {
            // empty ppdData means we're selecting new make/model
            if (Object.keys(ppdData).length === 0) {
                rbMake.checked = true
                ppdData.type = PM.PPDType.Custom
                ppdData.pcfile = ""
                makesList.currentIndex = 0
                makesList.positionViewAtBeginning()
                ppdData.make = model.data(model.index(0,0), Qt.DisplayRole).toString()
                printerModels.invalidateFilter()
                setCurrentMakeModel()
            } else {
                init()
            }
        }
    }

    // Try to load PPDModel if empty
    Component.onCompleted: {
        if (model.rowCount() === 0) {
            model.load()
        } else {
            loading = false
        }
    }

    Connections {
        target: model
        enabled: loading

        function onLoaded() {
            loading = false
        }
    }

    onClosed: destroy(10)

    standardButtons: Kirigami.Dialog.NoButton

    customFooterActions: [
        Kirigami.Action {
            text: i18n("Save")
            enabled: !loading
            icon.name: "dialog-ok-symbolic"
            onTriggered: {
                if (rbFile.checked && customFilename.text.length === 0) {
                    error.text = i18n("Select a PostScript Printer Description (PPD) file")
                    error.visible = true
                    fileButton.focus = true
                    return
                }

                if (rbMake.checked && makeModelList.currentIndex === -1) {
                    error.text = i18n("Printer model is required.  Please select a printer model.")
                    error.visible = true
                    makeModelList.focus = true
                    return
                }

                if (rbFile.checked) {
                    ppdData.file = customFilename.text
                }
                saveValues(ppdData)
                root.close()
            }
        }
    ]

    // Printer models are filtered by selected make and
    // optionally, by a second user filter
    KSFM.KSortFilterProxyModel {
        id: printerModels
        // This holds printer model lists, which are descendents
        // of the make items
        sourceModel: KSFM.KDescendantsProxyModel {
            sourceModel: root.model
        }

        filterRowCallback: (source_row, source_parent) => {
            const make = sourceModel.data(sourceModel.index(source_row, 0, source_parent)
                                          , PM.PPDModel.PPDMake)
            if (filterString.length === 0) {
                return make === ppdData.make
            }

            return make === ppdData.make
                && sourceModel.data(sourceModel.index(source_row, 0, source_parent)
                                    , PM.PPDModel.PPDMakeAndModel).toLowerCase().includes(filterString)
        }
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        enabled: !loading

        BannerWithTimer {
            id: error
            Layout.fillWidth: true
        }

        QQC2.ButtonGroup {
            id: buttonGroup
        }

        RowLayout {
            QQC2.RadioButton {
                id: rbMake
                text: i18nc("@option:radio", "Select Make and Model:")
                Layout.fillWidth: true
                onToggled: {
                    if (checked) {
                        ppdData.type = PM.PPDType.Custom
                    }
                }
                QQC2.ButtonGroup.group: buttonGroup
            }

            Kirigami.SearchField {
                enabled: rbMake.checked && makesList.count > 0
                placeholderText: i18nc("@info:placeholder", "Filter Models")
                onAccepted: {
                    printerModels.filterString = text.toLowerCase()
                    makeModelList.currentIndex = -1
                }
            }
        }

        RowLayout {
            enabled: rbMake.checked

            QQC2.ScrollView {
                Layout.fillHeight: true
                Layout.preferredWidth: Math.floor(root.width/3)
                clip: true

                contentItem: ListView {
                    id: makesList
                    clip: true
                    highlight: PlasmaExtras.Highlight {}
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0

                    model: root.model

                    delegate: PComp.ItemDelegate {
                        width: ListView.view.width
                        text: model?.display
                        icon.name: "system-user-prompt"

                        onClicked: {
                            ListView.view.currentIndex = index
                            ppdData.make = model.display
                            printerModels.invalidateFilter()
                            makeModelList.currentIndex = 0
                            makeModelList.positionViewAtBeginning()
                            setCurrentMakeModel()
                        }
                    }
                }
            }

            QQC2.ScrollView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true

                contentItem: ListView {
                    id: makeModelList
                    clip: true
                    highlight: PlasmaExtras.Highlight {}
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0

                    PComp.BusyIndicator {
                        running: loading
                        visible: loading
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        implicitWidth: Kirigami.Units.gridUnit * 6
                        implicitHeight: Kirigami.Units.gridUnit * 6
                    }

                    model: printerModels

                    delegate: PComp.ItemDelegate {
                        width: ListView.view.width
                        text: model?.ppdMakeModel
                        icon.name: "printer-symbolic"

                        onClicked: {
                            ListView.view.currentIndex = index
                            ppdData.file = ppdName
                            ppdData.makeModel = ppdMakeModel
                        }
                    }
                }
            }

        }

        QQC2.RadioButton {
            id: rbFile
            text: i18nc("@option:radio", "PPD file:")
            Layout.topMargin: Kirigami.Units.largeSpacing*3
            onToggled: {
                if (checked) {
                    ppdData.type = PM.PPDType.Manual
                    fileButton.focus = true
                }
            }
            QQC2.ButtonGroup.group: buttonGroup
        }

        RowLayout {
            enabled: rbFile.checked

            QQC2.TextField {
                id: customFilename
                readOnly: true
                Layout.fillWidth: true
            }

            QQC2.Button {
                id: fileButton
                text: i18nc("@action:button", "Select PPD File…")
                icon.name: "folder-print-symbolic"
                onClicked: fileDlg.open()
            }

            Dialogs.FileDialog {
                id: fileDlg
                acceptLabel: i18n("Select")
                nameFilters: [i18n("PostScript Printer Description Files (*.ppd)")]

                onAccepted: customFilename.text = currentFile
            }
        }
    }

 }
