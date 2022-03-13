/**
 * SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.10 as QQC2

import org.kde.kirigami 2.10 as Kirigami
import org.kde.kcm 1.2

SimpleKCM {

    title: i18n("Settings")

    Kirigami.FormLayout {
        id: form

        QQC2.CheckBox {
            text: i18n("Share printers connected to this system")
            checked: kcm.shareConnectedPrinters
            onToggled: {
                kcm.shareConnectedPrinters = checked
            }
        }

        QQC2.CheckBox {
            text: i18n("Allow printing from the Internet")
            enabled: kcm.shareConnectedPrinters
            checked: enabled && kcm.allowPrintingFromInternet
            onToggled: {
                kcm.allowPrintingFromInternet = checked
            }
        }

        QQC2.CheckBox {
            text: i18n("Allow remote administration")
            checked: kcm.allowRemoteAdmin
            onToggled: {
                kcm.allowRemoteAdmin = checked
            }
        }

        QQC2.CheckBox {
            text: i18n("Allow users to cancel any job (not just their own)")
            checked: kcm.allowUserCancelAnyJobs
            onToggled: {
                kcm.allowUserCancelAnyJobs = checked
            }
        }
    }
}
