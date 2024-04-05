/**
 SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
 SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

/**
 * Device setup for Network discovered printer devices,
 * ie. IPP/HTTP.
 *
 * Each printer device has recommended drivers.
 * (system-config-printer)
*/
BaseDevice {
    title: settings.value("printer-make-and-model")
    subtitle: settings.value("device-desc")
    showUri: false

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

        onDriverFallback: manualDriverSelect()
    }
}
