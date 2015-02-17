/*
 *   Copyright 2014 Jan Grulich <jgrulich@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import QtQuick.Controls 1.1 as Controls
import QtQuick.Layouts 1.1 as Layouts
import org.kde.plasma.core 2.0 as PlasmaCore


Item {
    id: configPage

    width: childrenRect.width
    height: childrenRect.height
    implicitWidth: mainColumn.implicitWidth
    implicitHeight: mainColumn.implicitHeight

    property alias cfg_allJobs: allJobs.checked
    property alias cfg_completedJobs: completedJobs.checked
    property alias cfg_activeJobs: activeJobs.checked

    Layouts.ColumnLayout {
        id: mainColumn

        Controls.ExclusiveGroup{
            id: jobsFilter
        }

        Controls.RadioButton {
            id: allJobs
            text: i18n("All jobs")
            exclusiveGroup: jobsFilter
        }

        Controls.RadioButton {
            id: completedJobs
            text: i18n("Completed jobs only")
            exclusiveGroup: jobsFilter
        }

        Controls.RadioButton {
            id: activeJobs
            text: i18n("Active jobs only")
            exclusiveGroup: jobsFilter
        }
    }
}

