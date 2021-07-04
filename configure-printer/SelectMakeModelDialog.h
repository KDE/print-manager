/*
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SELECTMAKEMODELDIALOG_H
#define SELECTMAKEMODELDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

#include "SelectMakeModel.h"

class SelectMakeModelDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectMakeModelDialog(const QString & make, const QString & makeModel, QWidget *parent = nullptr);
    ~SelectMakeModelDialog();

    SelectMakeModel * mainWidget() const;
private:
    SelectMakeModel * m_widget;
    QDialogButtonBox * m_bbox;
};

#endif // SELECTMAKEMODELDIALOG_H
