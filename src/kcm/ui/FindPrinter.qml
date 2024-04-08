/**
 SPDX-FileCopyrightText: 2023-2025 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.printmanager as PM
import org.kde.kcmutils as KCM
import org.kde.kitemmodels as KItemModels

pragma ComponentBehavior: Bound

KCM.AbstractKCM {
    id: root

    headerPaddingEnabled: false
    framedView: false

    property bool loading: false
    property bool showingManual: false
    property bool hasDetectedDevices: false

    property var newPrinterCallback

    // MFG:HP;MDL:ENVY 4520 series;CLS:PRINTER;DES:ENVY 4520 series;SN:TH6BN4M1390660;
    function parseDeviceId(devId: string, key: string) : var {
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
    }

    function setValues(configMap : var) {
        const cfgObj = {info: configMap["printer-info"]
                     , printerUri: configMap["device-uri"]
                     , location: configMap["printer-location"]
                     , "ppd-type": configMap["ppd-type"]
                     , "ppd-name": configMap["ppd-name"] ?? ""}

        if (configMap.hasOwnProperty("printer-model")) {
            cfgObj.printerName = configMap["printer-model"].replace(/ /g, "_")
        }

        // Set the PPD attrs
        const ppdObj = {make: configMap["printer-make"]
                     , makeModel: configMap["printer-make-and-model"]
                     , type: configMap["ppd-type"]
                     , file: configMap["ppd-name"] ?? ""}

        // if we have device file
        // strip out the base file name
        if (ppdObj.file) {
             cfgObj.kind = ppdObj.makeModel
             const i = ppdObj.file.lastIndexOf('/')
             if (i !== -1) {
                 ppdObj.pcfile = ppdObj.file.slice(-(ppdObj.file.length-i-1))
             } else {
                 ppdObj.pcfile = ppdObj.file
             }
        } else {
            cfgObj.kind = ""
            ppdObj.pcfile = ""
        }

        if (typeof newPrinterCallback == 'function') {
            newPrinterCallback(true, cfgObj, ppdObj)
        }

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
            deviceList.itemAtIndex(deviceList.currentIndex).clicked()
        }

    }

    title: i18nc("@title:window", "Set up a Printer Connection")

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Refresh")
            enabled: !root.loading
            icon.name: "view-refresh-symbolic"
            onTriggered: {
                root.showingManual = false
                devices.load()
            }
        },
        Kirigami.Action {
            text: root.showingManual
                  ? i18nc("@action:button", "Show Detected Devices")
                  : i18nc("@action:button", "Show Manual Options")
            icon.name: root.showingManual
                    ? "standard-connector-symbolic"
                    : "internet-services"
            visible: root.hasDetectedDevices
            enabled: !root.loading
            onTriggered: {
                root.showingManual = !root.showingManual
                deviceItems.invalidateFilter()
                deviceList.currentIndex = -1
                compLoader.sourceComponent = undefined

                if (!root.showingManual) {
                    root.setDeviceSelection()
                } else {
                    compLoader.sourceComponent = chooseManualComp
                }
            }
        }
    ]

    ConfigValues {
        id: settings
    }

    // Filter the descendants to exclude "null" deviceClass
    KItemModels.KSortFilterProxyModel {
        id: deviceItems
        sortRole: PM.DevicesModel.DeviceCategory

        // Descendants are the actual printer devices
        sourceModel: KItemModels.KDescendantsProxyModel {
            sourceModel: devices
        }

        filterRowCallback: (source_row, source_parent) => {
           const ndx = sourceModel.index(source_row, 0, source_parent)
           if (sourceModel.data(ndx, PM.DevicesModel.DeviceClass) === undefined) {
               return false
           }
           const cat = sourceModel.data(ndx, PM.DevicesModel.DeviceCategory)
           if (root.showingManual) {
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
            root.loading = true
            compLoader.selector = ""
            kcm.clearRemotePrinters()
            kcm.clearRecommendedDrivers()
            devices.update()
        }

        Component.onCompleted: load()

        onLoaded: {
            root.loading = false
            root.setDeviceSelection()
        }
    }

    Component {
        id: notAvailableComp

        Kirigami.PlaceholderMessage {
            icon.name: "package-available-locked"
            text: compLoader.info
            explanation: i18nc("@info:status", "This feature is not available (%1)", compLoader.selector)
        }
    }

    Component {
        id: noDevicesComp

        Kirigami.PlaceholderMessage {
            icon.name: "edit-none"
            text: i18nc("@info:status", "Unable to automatically discover any printing devices")
            explanation: i18nc("@info:usagetip", "Choose \"Refresh\" to try again or choose a manual configuration option from the list")
        }
    }

    Component {
        id: chooseManualComp

        Kirigami.PlaceholderMessage {
            icon.name: "edit-entry"
            text: i18nc("@info:usagetip", "Choose a manual configuration option from the list")
        }
    }

    RowLayout {
        spacing: 0
        anchors.fill: parent

        QQC2.ScrollView {
            Layout.fillHeight: true
            Layout.preferredWidth: Kirigami.Units.gridUnit*13
            clip: true
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false

            ListView {
                id: deviceList

                QQC2.BusyIndicator {
                    id: busyInd
                    running: root.loading
                    anchors.centerIn: parent
                    implicitWidth: Math.floor(parent.width/2)
                    implicitHeight: implicitWidth
                }

                clip: true
                currentIndex: -1

                activeFocusOnTab: true
                keyNavigationWraps: true

                model: deviceItems

                section {
                    property: "deviceCategory"
                    delegate: Kirigami.ListSectionHeader {
                        width: ListView.view.width
                        required property string section
                        text: section
                    }
                }

                delegate: Kirigami.SubtitleDelegate {

                    required property int index
                    required property string deviceId
                    required property string deviceUri
                    /**
                    * List of connection uris, typically only available
                    * when a device is directly connected
                    */
                    required property list<string> deviceUris

                    required property string deviceClass
                    required property string deviceInfo
                    required property string deviceMakeModel
                    required property string deviceDescription
                    required property string deviceLocation

                    width: ListView.view.width
                    visible: deviceClass !== undefined

                    text: deviceInfo.replace("Internet Printing Protocol", "IPP")
                    subtitle: deviceMakeModel.replace("Unknown", "")

                    icon.name: deviceId.length === 0
                               ? "internet-services"
                               : "printer-symbolic"

                    highlighted: ListView.view.currentIndex === index
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
                                                    , "printer-make": root.parseDeviceId(deviceId, "MFG")
                                                    , "printer-model": root.parseDeviceId(deviceId, "MDL")
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
        }

        Rectangle {
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            Kirigami.Theme.inherit: false
            color: Kirigami.Theme.backgroundColor
            Layout.fillWidth: true
            Layout.fillHeight: true

            Loader {
                id: compLoader
                active: !root.loading
                anchors.centerIn: parent

                // Force placeholders to format properly inside the Rect
                width: item instanceof Kirigami.PlaceholderMessage
                       ? parent.width - (Kirigami.Units.largeSpacing * 4)
                       : parent.width - Kirigami.Units.gridUnit

                height: item instanceof Kirigami.PlaceholderMessage
                        ? item.implicitHeight
                        : parent.height - Kirigami.Units.gridUnit

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
                        source = "components/ManualUri.qml"
                        break
                    case "network":
                        source = "components/Network.qml"
                        break
                    case "direct":
                        source = "components/Direct.qml"
                        break
                    case "lpd":
                        source = "components/Lpd.qml"
                        break
                    case "socket":
                        source = "components/Socket.qml"
                        break
                    case "smb":
                        source = "components/Smb.qml"
                        break
                    default:
                        sourceComponent = notAvailableComp
                    }
                }
            }
        }
    }
}
