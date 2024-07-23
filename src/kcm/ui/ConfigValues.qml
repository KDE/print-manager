/**
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

// Config values are {key: value} pairs
// In order to implement apply/reset features, keep
// the initial values immutable.  Changes are stored in pending.
QtObject {

    property var __initial: ({})
    property var pending: ({})

    // Check pending based on # of entries (>0) in the object
    // otherwise, compare stringify(initial === pending)
    property bool usePendingCount: true
    property bool hasPending: false

    // remove known unneeded keys (not needed by CUPS)
    function clean() {
        remove(["device-id", "device-class"
               , "device-desc", "device-uris"
               , "match", "printer-type"
               , "remote", "iconName"])
    }

    function init(map = {}) {
        __initial = map
        pending = Object.assign({}, __initial)
    }

    function clear() {
        console.log("set in clear")
        set({})
    }

    function remove(keylist) {
        if (keylist === undefined)
            return

        if (typeof keylist === "object") {
            // string array of keys
            keylist.forEach(k => delete pending[k])
        } else if (typeof keylist === "string") {
            // string
            delete pending[keylist]
        }
        
        console.log("set in remove")
        set()
    }

    function value(key : string) : var {
        if (key === undefined || key.length === 0) {
            return ""
        } else {
            return pending[key]
        }
    }

    function add(key : string, value) {
        console.log("add", key, value)
        if (key === undefined || key.length === 0 || value === undefined) {
            console.warn("KEY and VALUE must have values")
        } else {
            const obj = {}
            obj[key] = value
            console.log("set in add")
            set(obj)
        }
    }

    // if obj and has fields, assign obj over pending
    // otherwise, clear pending
    // always set pending flag
    function set(obj) {
        console.log("set", JSON.stringify(obj))
        if (obj !== undefined && typeof obj === "object") {
            if (Object.keys(obj).length > 0) {
                Object.assign(pending, obj)
            } else {
                pending = {}
            }
        }

        console.log("use pending count", usePendingCount)
        if (usePendingCount) {
            hasPending = Object.keys(pending).length > 0
            console.log("hasPending", hasPending)
        } else {
            hasPending = JSON.stringify(__initial) !== JSON.stringify(pending)
            console.log("hasPending", hasPending)
        }
    }

    function reset() {
        if (usePendingCount) {
            pending = {}
        } else {
            pending = Object.assign({}, __initial)
        }
        console.log("set in reset")
        set()
    }
}
