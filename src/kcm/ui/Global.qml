/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 
import QtQuick.Layouts 
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    title: i18n("Global Settings")

    ColumnLayout {
        anchors.centerIn: parent

        QQC2.Switch {
            text: i18n("Share printers connected to this system")
            checked: kcm.shareConnectedPrinters
            onToggled: {
                kcm.shareConnectedPrinters = checked
            }
        }

        QQC2.Switch {
            text: i18n("Allow printing from the Internet")
            enabled: kcm.shareConnectedPrinters
            checked: enabled && kcm.allowPrintingFromInternet
            onToggled: {
                kcm.allowPrintingFromInternet = checked
            }
        }

        QQC2.Switch {
            text: i18n("Allow remote administration")
            checked: kcm.allowRemoteAdmin
            onToggled: {
                kcm.allowRemoteAdmin = checked
            }
        }

        QQC2.Switch {
            text: i18n("Allow users to cancel any job (not just their own)")
            checked: kcm.allowUserCancelAnyJobs
            onToggled: {
                kcm.allowUserCancelAnyJobs = checked
            }
        }
    }
}
