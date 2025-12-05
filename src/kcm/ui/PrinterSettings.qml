/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 
import QtQuick.Layouts 
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.kitemmodels as KItemModels
import org.kde.plasma.printmanager as PM

pragma ComponentBehavior: Bound

KCM.AbstractKCM {
    id: root
    headerPaddingEnabled: false

    // Used to elevate priviledges
    property var authFunction

    // Add mode means adding a new printer/group
    property bool addMode: false
    property var modelData
    // Printer ppd attributes
    property var ppd

    property PM.PrinterModel printerModel
    property PM.PPDModel ppdModel

    function openMakeModelDlg() {
        const dlg = mmComp.createObject(root)
        dlg.open()
    }

    function printerNameIsUnique(name: string) : bool {
        for (let i=0, len=printerModel.rowCount(); i<len; ++i) {
            if (printerModel.data(printerModel.index(i,0), PM.PrinterModel.DestName).toString() === name)
                return false
        }
        return true
    }

    title: {
        if (addMode) {
            return modelData.isClass
                    ? i18nc("@title:window", "Add Group")
                    : i18nc("@title:window", "Add Printer")
        } else {
            const locStr = modelData.location && printerModel.displayLocationHint
                            ? " (%1)".arg(modelData.location)
                            : ""
            return modelData.info + locStr
        }
    }

    actions: [
        Kirigami.Action {
            text: i18nc("@action", "Configure Media Settings…")
            icon.name: "configure-symbolic"
            visible: !root.addMode
            onTriggered: PM.ProcessRunner.configurePrinter(root.modelData.printerName)
        }, Kirigami.Action {
            text: i18n("Remove")
            icon.name: "edit-delete-remove-symbolic"
            visible: !root.addMode
            onTriggered: removeLoader.active = true
        }
    ]

    header: BannerWithTimer {
        id: msgBanner
        position: Kirigami.InlineMessage.Position.Header
    }

    footer: RowLayout {
        Layout.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.UrlButton {
            text: i18nc("@action:button", "CUPS Printers Overview Help")
            url: "http://localhost:631/help/overview.html"
            padding: Kirigami.Units.largeSpacing
        }

        Kirigami.UrlButton {
            text: i18nc("@action:button", "Printer/Device Admin Page")
            visible: !root.modelData.isClass && url !== ""
            url: {
                try {
                    const url = new URL(devUri.text)
                    if (url.hostname.length > 0) {
                        return `http://${url.hostname}`
                    }
                } catch(e) {
                }
                return ""
            }
        }

        Item { Layout.fillWidth: true }

        QQC2.Button {
            text: root.addMode
                  ? i18nc("@action:button Add printer", "Add")
                  : i18nc("@action:button Apply changes", "Apply")
            icon.name: "dialog-ok-apply"
            enabled: config.hasPending

            onClicked: {
                if (root.addMode) {
                    if (queueName.text.length === 0) {
                        queueName.focus = true
                        msgBanner.text = i18nc("@info:status", "Queue name is required. Enter a unique queue name.")
                        msgBanner.visible = true
                        return
                    } else if (!root.printerNameIsUnique(queueName.text)) {
                        queueName.focus = true
                        msgBanner.text = i18nc("@info:status", "Queue name must be unique. Enter a unique queue name.")
                        msgBanner.visible = true
                        return
                    }

                    if (!root.modelData.isClass) {
                        if (driver.text.length === 0) {
                            driverSelect.focus = true
                            msgBanner.text = i18nc("@info:status", "Make/Model is required. Choose \"Select\" to pick Make/Model")
                            msgBanner.visible = true
                            return
                        }
                    }

                    config.add("add", true)
                }

                config.hasPending = false
                msgBanner.text = i18nc("@info:status", "Configuring %1, please wait…", !modelData.isClass ? "printer" : "group")
                msgBanner.showCloseButton = false
                msgBanner.type = Kirigami.MessageType.Positive
                msgBanner.visible = true
                kcm.savePrinter(queueName.text, config.pending, root.modelData.isClass)
            }
        }
    }

    Component.onCompleted: {
        if (!modelData.isClass) {
            if (addMode) {
                config.set({"ppd-name": modelData["ppd-name"]
                           , "ppd-type": modelData["ppd-type"]})

                // there is no ppd info, so show make/model selection dialog initially
                if (config.value("ppd-name") === "") {
                    openMakeModelDlg()
                }
            } else {
                // IFF an existing printer, ask the kcm to load the ppd
                kcm.loadPrinterPPD(modelData.printerName)
            }
        }
    }

    // Each item stores its corresponding CUPS field name in objectName
    // This is then used to generate the "pending" changes map
    ConfigValues {
        id: config
    }

    Connections {
        id: kcmConn
        target: kcm

        property bool removing: false

        function onRequestError(errorMessage) {
            msgBanner.reset()
            if (removing) {
                removing = false
                msgBanner.text = i18n("Failed to remove the printer: %1", errorMessage)
            } else {
                msgBanner.text = errorMessage
            }
            msgBanner.visible = true
            config.clear()
        }

        function onPpdLoaded(printerPPD) {
            root.ppd = printerPPD
        }
        
    }

    Loader {
        id: removeLoader
        active: false

        sourceComponent: Kirigami.PromptDialog {
            id: prompt
            parent: root

            dialogType: Kirigami.PromptDialog.Warning

            Component.onCompleted: open()
            onClosed: removeLoader.active = false

            title: root.modelData.isClass ? i18nc("@title:window", "Remove Group?")
                                     : i18nc("@title:window", "Remove Printer?")
            subtitle: i18nc("@info %1 is the name of a printer or printer group",
                            "'%1' will be removed.",
                            root.modelData.info)

            standardButtons: Kirigami.Dialog.NoButton

            customFooterActions: [
                Kirigami.Action {
                    text: root.modelData.isClass ? i18nc("@action:button", "Remove Group")
                                            : i18nc("@action:button", "Remove Printer")
                    icon.name: "edit-delete-remove-symbolic"
                    onTriggered: {
                        kcmConn.removing = true
                        kcm.removePrinter(root.modelData.printerName)
                        prompt.close()
                    }
                },
                Kirigami.Action {
                    text: i18n("Cancel")
                    icon.name: "dialog-cancel-symbolic"
                    onTriggered: prompt.close()
                }
            ]
        }

    }

    Component {
        id: mmComp

        MakeModel {
            anchors.centerIn: parent
            implicitWidth: Math.ceil(parent.width*.85)
            implicitHeight: Math.ceil(parent.height*.85)

            model: root.ppdModel
            ppdData: Object.assign({}, root.ppd)
            printerSettings: root.modelData

            onSaveValues: ppdMap => {
                Object.assign(root.ppd, ppdMap)
                driver.text = root.ppd.type !== PM.PPDType.Manual
                          ? root.ppd.makeModel
                          : root.ppd.file

                if (root.ppd.file.length > 0) {
                    config.set({"ppd-type": root.ppd.type
                               , "ppd-name": root.ppd.file})
                    const i = root.ppd.file.lastIndexOf('/')
                    if (i !== -1) {
                        root.ppd.pcfile = root.ppd.file.slice(-(root.ppd.file.length-i-1))
                    } else {
                        root.ppd.pcfile = root.ppd.file
                    }
                } else {
                    config.remove(["ppd-name", "ppd-type"])
                    root.ppd.pcfile = ""
                }
            }
        }
    }

    component PrinterField: QQC2.TextField {
        Layout.fillWidth: true
        property string orig
        text: orig

        Component.onCompleted: {
            if (root.addMode) {
                config.add(objectName, text)
            }
        }

        onEditingFinished: {
            if (!root.addMode) {
                config.remove(objectName)
            }

            if (text !== orig) {
                config.add(objectName, text)
            }
        }
    }

    component PrinterOption: QQC2.CheckBox {
        property bool orig
        checked: orig

        Component.onCompleted: {
            if (root.addMode) {
                config.add(objectName, checked)
            }
        }

        onToggled: {
            if (!root.addMode) {
                config.remove(objectName)
            }

            if (checked !== orig) {
                config.add(objectName, checked)
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: root.modelData.isClass ? "folder-print" : root.modelData.iconName
                Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
                Layout.preferredHeight: Layout.preferredWidth
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    text: root.modelData.info ?? ""
                    visible: !root.addMode
                    level: 3
                    type: Kirigami.Heading.Type.Primary
                }

                Kirigami.Heading {
                    text: root.modelData.kind.replace("Class", "Group")
                    visible: !root.addMode
                    level: 5
                    type: Kirigami.Heading.Type.Secondary
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    PrinterOption {
                        objectName: "isDefault"
                        text: i18nc("@action:check Set default printer", "Default printer")
                        orig: root.modelData.isDefault
                        visible: root.addMode || root.printerModel.rowCount() > 1
                        // CUPS treats default printer independently from other printer attributes
                        // Also, CUPS makes sure it's exclusive, only one can be default and there is
                        // no api for "Not default".

                        // Therefore, if a printer is default, don't allow change for that printer,
                        // only allow it to be set true for a printer that is not default.
                        // However, when adding a new printer, allow the option to set default
                        enabled: root.addMode || !root.modelData.isDefault
                        checked: root.modelData.isDefault
                    }

                    Kirigami.ContextualHelpButton {
                        toolTipText: i18nc("@info", "To change the default printer, set another printer as the default.") 
                        visible: root.modelData.isDefault
                    }
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    PrinterOption {
                        objectName: "printer-is-shared"
                        text: root.modelData.isClass
                              ? i18nc("@action:check", "Share this group")
                              : i18nc("@action:check", "Share this printer")
                        enabled: kcm.serverSettingsLoaded && kcm.shareConnectedPrinters
                        orig: root.modelData.isShared
                    }

                    QQC2.ToolButton {
                        icon.name: "unlock-symbolic"
                        visible: typeof root.authFunction === "function" && !kcm.serverSettingsLoaded
                        onClicked: root.authFunction()

                        QQC2.ToolTip.text: i18nc("@info", "Click here to elevate privileges to unlock this feature")
                        QQC2.ToolTip.visible: hovered || activeFocus
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                    }
                }

                PrinterOption {
                    objectName: "printer-is-accepting-jobs"
                    text: i18nc("@action:check", "Accepting print jobs")
                    orig: root.modelData.isAcceptingJobs
                }
            }
        }

        // Marker (ink) status
        QQC2.ScrollView {
            visible: markersView.count > 0
            Layout.fillWidth: true
            Layout.maximumHeight: Math.floor(root.height/4)
            Layout.preferredHeight: contentHeight + Kirigami.Units.smallSpacing

            Component.onCompleted: {
                if (background) {
                    background.visible = true;
                }
            }

            contentItem: ListView {
                id: markersView
                model: !root.addMode && !root.modelData.isClass ? root.modelData.markers["marker-names"] : null
                clip: true
                delegate: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    required property var modelData
                    required property int index

                    QQC2.Label {
                        text: modelData
                        horizontalAlignment: Text.AlignRight
                        Layout.minimumWidth: Kirigami.Units.gridUnit*10
                    }

                    QQC2.ProgressBar {
                        from: 0
                        to: 100
                        value: root.modelData.markers["marker-levels"][index]
                        Kirigami.Theme.highlightColor: root.modelData.markers["marker-colors"][index]
                        Kirigami.Theme.inherit: false
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // Maint actions
        RowLayout {
            visible: !root.addMode
            Layout.topMargin: Kirigami.Units.largeSpacing

            QQC2.Button {
                text: i18nc("@action:button", "Print Test Page")
                icon.name: "document-print-preview-symbolic"
                onClicked: kcm.printTestPage(root.modelData.printerName, root.modelData.isClass)
            }

            QQC2.Button {
                text: i18nc("@action:button", "Print Self-Test Page")
                icon.name: "document-print-preview-symbolic"
                visible: root.modelData.commands.indexOf("PrintSelfTestPage") !== -1
                onClicked: kcm.printSelfTestPage(root.modelData.printerName)
            }

            QQC2.Button {
                text: i18nc("@action:button", "Clean Print Heads")
                icon.name: "edit-clear-all-symbolic"
                visible: root.modelData.commands.indexOf("Clean") !== -1
                onClicked: kcm.cleanPrintHeads(root.modelData.printerName)
            }
        }

        Kirigami.Separator {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
        }

        GridLayout {
            columns: 2
            columnSpacing: Kirigami.Units.gridUnit

            QQC2.Label {
                text: i18nc("@label:textbox", "Queue Name:")
                Layout.alignment: Qt.AlignRight
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                PrinterField {
                    id: queueName
                    objectName: "printer-name"
                    orig: root.modelData.printerName
                    enabled: root.addMode
                    validator: RegularExpressionValidator { regularExpression: /[^/#\\ ]*/ }
                }

                Kirigami.ContextualHelpButton {
                    visible: root.modelData.isClass
                    toolTipText: xi18nc("@info:whatsthis", `A <interface>printer group</interface> is used to pool printing resources.
                                        Member printers can be added to a group and print jobs sent to that group
                                        will be dispatched to the appropriate printer.`)
                }
            }


            QQC2.Label {
                text: i18nc("@label:textbox", "Description:")
                Layout.alignment: Qt.AlignRight
            }

            PrinterField {
                id: queueInfo
                objectName: "printer-info"
                readOnly: root.modelData.remote
                orig: root.modelData.info ?? ""
            }

            QQC2.Label {
                text: i18nc("@label:textbox", "Location:")
                Layout.alignment: Qt.AlignRight
            }

            PrinterField {
                id: location
                objectName: "printer-location"
                readOnly: root.modelData.remote
                orig: root.modelData.location ?? ""
            }

            QQC2.Label {
                text: i18nc("@label:textbox", "Connection:")
                Layout.alignment: Qt.AlignRight
                visible: !root.modelData.isClass
            }

            PrinterField {
                id: devUri
                visible: !root.modelData.isClass
                objectName: "device-uri"
                orig: root.modelData.printerUri ?? ""
                readOnly: root.modelData.remote
            }

            QQC2.Label {
                text: i18nc("@label:listbox", "Member Printers:")
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
                visible: root.modelData.isClass
            }

            // Printer Class member list
            Loader {
                active: root.modelData.isClass
                visible: active
                Layout.fillHeight: true
                Layout.preferredWidth: Math.round(root.width/2)

                sourceComponent: QQC2.ScrollView {

                    Component.onCompleted: {
                        if (background) {
                            background.visible = true
                        }
                    }

                    contentItem: ListView {
                        id: memberList
                        // cups key for the member list
                        objectName: "member-uris"
                        clip: true

                        property bool showClasses: false

                        model: KItemModels.KSortFilterProxyModel {
                            sourceModel: root.printerModel

                            filterRowCallback: (source_row, source_parent) => {
                                const ndx = sourceModel.index(source_row, 0, source_parent)
                                const pn = sourceModel.data(ndx, PM.PrinterModel.DestName)

                                if (!memberList.showClasses) {
                                    const isClass = sourceModel.data(ndx, PM.PrinterModel.DestIsClass)
                                    const isRemote = sourceModel.data(ndx, PM.PrinterModel.DestRemote)
                                    if (isClass || isRemote) {
                                        return false
                                    }
                                }
                                return pn !== root.modelData.printerName
                            }
                        }

                        // TODO: Seems to be a timing issue with the delegates and the KSFM.
                        // They're not available right away, so push the setting of
                        // check state a bit later.
                        Component.onCompleted: {
                            if (root.modelData.memberNames.length > 0) {
                                checkTimer.start()
                            }
                        }

                        Timer {
                            id: checkTimer
                            interval: 100; repeat: true; running: false
                            onTriggered: {
                                if (memberList.count > 0) {
                                    stop()
                                    memberList.setChecked()
                                }
                            }
                        }

                        // CUPS strips the queue name from the URI
                        // so for display, compare the queue name.
                        function setChecked() {
                            for (let i=0; i<count; ++i) {
                                const cb = itemAtIndex(i)
                                if (cb instanceof Kirigami.CheckSubtitleDelegate)
                                    cb.checked = cb?.visible
                                        && root.modelData.memberNames.includes(cb.objectName)
                            }
                        }

                        // For save, use the full URI
                        function getChecked(keysOnly: bool) : list<var> {
                            let ret = []
                            for (let i=0; i<count; ++i) {
                                const item = itemAtIndex(i)
                                if (item.checked) {
                                    ret.push(keysOnly ? item.objectName : item.uriSupported)
                                }
                            }
                            return ret
                        }

                        function hasChanges() : bool {
                            let changed = false
                            const vals = getChecked(true)
                            if (vals.length !== root.modelData.memberNames.length
                                    || JSON.stringify(vals) !== JSON.stringify(root.modelData.memberNames)) {
                                changed = true
                            }

                            return changed
                        }

                        delegate: Kirigami.CheckSubtitleDelegate {

                            required property string printerName
                            required property string info
                            required property string uriSupported

                            width: ListView.view.width
                            icon.width: 0
                            objectName: printerName

                            text: info
                            subtitle: printerName

                            // if there are changes, send checked list
                            // if there are changes and nothing is checked, send empty object
                            // if there are NO changes, just send the checked list
                            onToggled: {
                                const cfg = {}
                                if (memberList.hasChanges()) {
                                    const list = memberList.getChecked()
                                    if (list.length > 0)
                                        cfg[memberList.objectName] = list
                                } else {
                                    cfg[memberList.objectName] = memberList.getChecked()
                                }

                                // an empty member list implies the class should be removed
                                if (Object.keys(cfg).length > 0) {
                                    config.set(cfg)
                                } else {
                                    config.remove(memberList.objectName)
                                }

                            }
                        }
                    }
                }

            }

            QQC2.Label {
                text: i18nc("@label:textbox", "Make/Model:")
                Layout.alignment: Qt.AlignRight
                visible: !root.modelData.isClass
            }

            RowLayout {
                visible: !root.modelData.isClass

                QQC2.Label {
                    id: driver
                    text: root.modelData.kind ?? ""
                }

                QQC2.Button {
                    id: driverSelect
                    text: i18nc("@action:button Select printer make/model", "Select…")
                    icon.name: "printer-symbolic"
                    enabled: !root.modelData.remote

                    onClicked: {
                        root.openMakeModelDlg()
                    }
                }
            }

        }

    }

}
