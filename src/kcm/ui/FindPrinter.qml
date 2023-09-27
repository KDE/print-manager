/**
 SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.plasma.components as PComp
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PM
import org.kde.kitemmodels as KSFM
import org.kde.plasma.extras as PlasmaExtras

Kirigami.Dialog {
    id: root

    property bool loading: false
    property bool showingManual: false
    property bool hasDetectedDevices: false

    // MFG:HP;MDL:ENVY 4520 series;CLS:PRINTER;DES:ENVY 4520 series;SN:TH6BN4M1390660;
    function parseDeviceId(devId: string, key: string) {
        if (devId === undefined) {
            return ""
        }

        // if no key, return the array
        const arr = devId.split(";")
        if (key === undefined) {
            return arr
        }

        // otherwise, return key[value]
        for (let i=0, len=arr.length; i<len; ++i) {
            const a = arr[i].split(":")
            if (a[0] === key)
                return a[1]
        }

        return ""
    }

    // Remove the ppd settings to force manual make/model driver selection
    function manualDriverSelect() {
        settings.remove("ppd-name")
        settings.add("ppd-type", PM.PPDType.Custom)
        root.setValues(settings.pending)
        close()
    }

    // Find the first direct device and network device
    // note, we're looping the decscendent filter model
    function setDeviceSelection() {
        let directNdx = -1
        let netNdx = -1
        for (let i=0, len=deviceItems.rowCount(); i<len; ++i) {
            const ndx = deviceItems.mapToSource(deviceItems.index(i,0))
            const cls = deviceItems.sourceModel.data(ndx, PM.DevicesModel.DeviceClass)
            const devId = deviceItems.sourceModel.data(ndx, PM.DevicesModel.DeviceId)
            if (cls.toString() === "direct") {
                directNdx = i
            } else if (cls.toString() === "network" && devId.length > 0) {
                netNdx = i
            }
            if (netNdx >= 0 && directNdx >= 0)
                break
        }

        // Did we actually find device, either direct or network?
        if (directNdx === -1 && netNdx === -1) {
            compLoader.sourceComponent = noDevicesComp
            hasDetectedDevices = false
            showingManual = true
            deviceItems.invalidateFilter()
        } else {
            hasDetectedDevices = true
            // by default, select direct connect printer
            deviceList.currentIndex = directNdx !== -1 ? directNdx : netNdx
            deviceList.itemAtIndex(deviceList.currentIndex).onClicked()
        }

    }

    signal setValues(var values)

    title: i18nc("@title:window", "Set up a Printer Connection")

    standardButtons: Kirigami.Dialog.NoButton

    customFooterActions: [
        Kirigami.Action {
            text: showingManual
                  ? i18nc("@action:button", "Show Detected Devices")
                  : i18nc("@action:button", "Show Manual Options")
            icon.name: showingManual
                    ? "standard-connector-symbolic"
                    : "internet-services"
            visible: hasDetectedDevices
            onTriggered: {
                showingManual = !showingManual
                deviceItems.invalidateFilter()
                deviceList.currentIndex = -1
                compLoader.sourceComponent = undefined

                if (!showingManual) {
                    setDeviceSelection()
                } else {
                    compLoader.sourceComponent = chooseManualComp
                }
            }
        },
        Kirigami.Action {
            text: i18n("Refresh")
            enabled: !loading
            icon.name: "view-refresh-symbolic"
            onTriggered: {
                showingManual = false
                devices.load()
            }
        }
    ]

    footerLeadingComponent: Kirigami.UrlButton {
        text: i18n("CUPS Network Printers Help")
        url: "http://localhost:631/help/network.html"
        padding: Kirigami.Units.largeSpacing
    }

    onClosed: destroy(10)

    ConfigValues {
        id: settings
    }

    // Filter the descendants to exclude "null" deviceClass
    KSFM.KSortFilterProxyModel {
        id: deviceItems
        sortRole: PM.DevicesModel.DeviceCategory

        // Descendants are the actual printer devices
        sourceModel: KSFM.KDescendantsProxyModel {
            sourceModel: devices
        }

        filterRowCallback: (source_row, source_parent) => {
           const ndx = sourceModel.index(source_row, 0, source_parent)
           if (sourceModel.data(ndx, PM.DevicesModel.DeviceClass) === undefined) {
               return false
           }
           const cat = sourceModel.data(ndx, PM.DevicesModel.DeviceCategory)
           if (showingManual) {
               return cat === "Manual"
           } else {
               return cat !== "Manual"
           }

        }
    }

    // Two-level QSIM, top level is "device category" (Qt.UserRole)
    PM.DevicesModel {
        id: devices

        function load() {
            loading = true
            kcm.clearRemotePrinters()
            kcm.clearRecommendedDrivers()
            update()
        }

        Component.onCompleted: load()

        onLoaded: {
            loading = false
            setDeviceSelection()
        }
    }

    Component {
        id: uriComp

        BaseDevice {
            id: uriItem

            title: compLoader.info
            subtitle: i18nc("@title:group", "Find remote printing devices")
            helpText: i18nc("@info:usagetip", "Enter the address of the remote host/device")
            icon.source: "internet-services"

            property var examples: [
                "ipp://ip-addr/ipp/print",
                "ipp://ip-addr-or-hostname/printers/name",
                "ipps://ip-addr/ipp/print",
                "ipps://ip-addr-or-hostname/printers/name",
                "http://ip-addr-or-hostname:port-number/printers/name",
                "lpd://ip-addr/queue",
                "lpd://ip-addr/queue?format=l",
                "lpd://ip-addr/queue?format=l&reserve=rfc1179",
                "socket://ip-addr",
                "socket://ip-addr:port-number/?...",
                "socket://ip-addr/?contimeout=30",
                "socket://ip-addr/?waiteof=false",
                "socket://ip-addr/?contimeout=30&waiteof=false"
            ]

            actions: [
                Kirigami.Action {
                    text: i18nc("@action:button", "Select Printer")
                    enabled: list.currentIndex !== -1
                    icon.name: "dialog-ok-symbolic"
                    onTriggered: {
                        settings.set(kcm.remotePrinters[list.currentIndex])
                        manualDriverSelect()
                    }
                }
            ]

            Component.onCompleted: {
                connSearch.text = compLoader.selector !== "other"
                        ? compLoader.selector + "://"
                        : "ipp://"
                connSearch.forceActiveFocus()
            }

            Connections {
                target: kcm

                function onRemotePrintersLoaded() {
                    if (list.count > 0) {
                        list.itemAtIndex(0).onClicked()
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                QQC2.Label {
                    text: i18nc("@label:textbox", "Uri:")
                }

                Kirigami.SearchField {
                    id: connSearch
                    focus: true
                    delaySearch: true
                    autoAccept: false
                    placeholderText: i18n("Enter host URI")
                    Layout.fillWidth: true

                    KeyNavigation.left: uriItem.parent
                    KeyNavigation.right: list

                    KeyNavigation.down: list

                    KeyNavigation.backtab: KeyNavigation.left
                    KeyNavigation.tab: KeyNavigation.right

                    onAccepted: {
                        if (text.length > 0)
                            kcm.getRemotePrinters(text, compLoader.selector)
                        else
                            kcm.clearRemotePrinters()
                    }
                }
            }

            // remote printer list
            QQC2.ScrollView {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                Layout.fillHeight: true

                contentItem: ListView {
                    id: list
                    currentIndex: -1
                    highlight: PlasmaExtras.Highlight {}
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0

                    activeFocusOnTab: true
                    keyNavigationWraps: true

                    KeyNavigation.up: connSearch
                    KeyNavigation.backtab: uriItem.parent
                    Keys.onUpPressed: event => {
                        if (currentIndex === 0) {
                            currentIndex = -1
                        }
                        event.accepted = false
                    }

                    model: kcm.remotePrinters

                    delegate: Kirigami.SubtitleDelegate {
                        width: ListView.view.width
                        text: modelData["printer-info"]
                        subtitle: modelData["printer-name"]
                        icon.name: modelData.remote
                                    ? "folder-network-symbolic"
                                    : modelData["printer-is-class"] ? "folder" : modelData.iconName

                        onClicked: {
                            ListView.view.currentIndex = index
                        }
                    }
                }
            }

            // example network addresses
            QQC2.ScrollView {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                Layout.fillHeight: true

                contentItem: ListView {
                    clip: true
                    model: examples

                    headerPositioning: ListView.OverlayHeader
                    header: Kirigami.InlineViewHeader {
                        text: i18nc("@info:usagetip", "Example Addresses")
                        implicitWidth: ListView.view.width
                    }

                    delegate: QQC2.ItemDelegate {
                        implicitWidth: ListView.view.width
                        text: modelData
                        onClicked: connSearch.text = modelData
                    }
                }
            }

        }
    }

    Component {
        id: networkComp

        BaseDevice {
            title: settings.value("printer-make-and-model")
            subtitle: settings.value("device-desc")

            Component.onCompleted: {
                drivers.load(settings.value("device-id")
                              , settings.value("printer-make-and-model")
                              , settings.value("device-uri"))
            }

            // Recommended Driver list
            Drivers {
                id: drivers

                onSelected: driverMap => {
                    settings.set(driverMap)
                    root.setValues(settings.pending)
                    close()
                }
            }
        }
    }

    Component {
        id: directComp

        BaseDevice {
            title: settings.value("printer-make-and-model")
            subtitle: settings.value("device-desc")
            helpText: i18nc("@info:usagetip", "Choose a device connection")

            // Connection list
            QQC2.ScrollView {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                Layout.fillHeight: true

                contentItem: ListView {
                    id: directlist
                    highlight: PlasmaExtras.Highlight {}
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0

                    activeFocusOnTab: true
                    keyNavigationWraps: true

                    KeyNavigation.backtab: root.parent
                    Keys.onUpPressed: event => {
                        if (currentIndex === 0) {
                            currentIndex = -1;
                        }
                        event.accepted = false;
                    }

                    model: settings.value("device-uris")

                    delegate: PComp.ItemDelegate {
                        width: ListView.view.width
                        text: devices.uriDevice(modelData)
                        icon.name: "standard-connector-symbolic"

                        Component.onCompleted:  {
                            if (index === 0)
                                onClicked()
                        }

                        onClicked: {
                            ListView.view.currentIndex = index
                            settings.add("device-uri", modelData)
                            drivers.load(settings.value("device-id")
                                          , settings.value("printer-make-and-model")
                                          , modelData)
                        }
                    }
                }
            }

            // Recommended Driver list
            Drivers {
                id: drivers

                onSelected: driverMap => {
                    settings.set(driverMap)
                    root.setValues(settings.pending)
                    close()
                }
            }
        }
    }

    Component {
        id: lpdComp
        NotAvailable {}
    }

    Component {
        id: socketComp
        NotAvailable {}
    }

    Component {
        id: serialComp
        NotAvailable {}
    }

    Component {
        id: smbComp
        NotAvailable {}
    }

    Component {
        id: naComp
        NotAvailable {}
    }

    Component {
        id: noDevicesComp

        Kirigami.PlaceholderMessage {
            text: i18nc("@info:status", "Unable to automatically discover any printing devices")
            explanation: i18nc("@info:usagetip", "Choose \"Refresh\" to try again or choose a manual configuration option from the list")
            Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing * 4
        }
    }

    Component {
        id: chooseManualComp

        Kirigami.PlaceholderMessage {
            text: i18nc("@info:usagetip", "Choose a manual configuration option from the list")
            Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing * 4
        }
    }

    component NotAvailable: ColumnLayout {
        // This is inside a Loader that is fillW/fillH: true
        // pad with "spacers" top/bottom to force centering
        Item { Layout.fillHeight: true }

        Kirigami.PlaceholderMessage {
            icon.name: "package-available-locked"
            text: compLoader.info
            explanation: i18nc("@info:status", "This feature is not yet available (%1)", compLoader.selector)
            Layout.maximumWidth: parent.width - Kirigami.Units.largeSpacing * 4
        }

        Item { Layout.fillHeight: true }
    }

    contentItem: RowLayout {
        spacing: 0

        QQC2.ScrollView {
            Layout.fillHeight: true
            Layout.preferredWidth: Kirigami.Units.gridUnit*13
            clip: true

            contentItem: ListView {
                id: deviceList

                PComp.BusyIndicator {
                    id: busyInd
                    running: loading
                    anchors.centerIn: parent
                    implicitWidth: Math.floor(parent.width/2)
                    implicitHeight: implicitWidth
                }

                clip: true
                currentIndex: -1
                highlight: PlasmaExtras.Highlight {}
                highlightMoveDuration: 0
                highlightResizeDuration: 0

                activeFocusOnTab: true
                keyNavigationWraps: true

                model: deviceItems

                section {
                    property: "deviceCategory"
                    delegate: Kirigami.ListSectionHeader {
                        width: ListView.view.width
                        required property string section
                        label: section
                    }
                }

                delegate: Kirigami.SubtitleDelegate {
                    width: ListView.view.width
                    visible: deviceClass !== undefined

                    text: deviceInfo.replace("Internet Printing Protocol", "IPP")
                    subtitle: deviceMakeModel.replace("Unknown", "")

                    icon.name: deviceId.length === 0
                               ? "internet-services"
                               : "printer-symbolic"

                    onClicked: {
                        ListView.view.currentIndex = index
                        compLoader.selector = ""
                        compLoader.info = ""
                        settings.clear()

                        if (deviceUri && deviceUri.length > 0) {
                            compLoader.info = deviceInfo
                            if (deviceId && deviceId.length > 0) {
                                if (deviceClass === "file") {
                                    compLoader.selector = deviceUri.replace(/:\//g,'')
                                } else {
                                    // a printer device
                                    settings.set({"device-id": deviceId
                                                    , "device-uri": deviceUri
                                                    , "device-uris": deviceUris
                                                    , "device-class": deviceClass
                                                    , "device-desc": deviceDescription
                                                    , "printer-info": deviceInfo
                                                    , "printer-make": parseDeviceId(deviceId, "MFG")
                                                    , "printer-model": parseDeviceId(deviceId, "MDL")
                                                    , "printer-make-and-model": deviceMakeModel
                                                    , "printer-location": deviceLocation
                                                    , "ppd-type": PM.PPDType.Custom
                                                 })
                                    compLoader.selector = deviceClass
                                }

                            } else {
                                // a category item
                                compLoader.selector = deviceUri
                            }
                        }
                    }
                }
            }
        }

        Kirigami.Separator {
            Layout.fillHeight: true
            width: 1
        }

        Loader {
            id: compLoader
            active: !loading

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Kirigami.Units.largeSpacing

            property string selector: ""
            property string info: ""

            onSelectorChanged: {
                switch (selector) {
                case "other":
                case "ipp":
                case "ipps":
                case "http":
                case "https":
                case "scsi":
                case "cups-brf":
                    sourceComponent = uriComp
                    break
                case "network":
                    sourceComponent = networkComp
                    break
                case "direct":
                    sourceComponent = directComp
                    break
                case "lpd":
                    sourceComponent = lpdComp
                    break
                case "socket":
                    sourceComponent = socketComp
                    break
                case "smb":
                    sourceComponent = sambaComp
                    break
                case "serial":
                    sourceComponent = serialComp
                    break
                default:
                    sourceComponent = naComp
                }
            }
        }

    }

}
