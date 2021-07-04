/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti
    dantti12@gmail.com

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

#ifndef CHOOSE_SERIAL_H
#define CHOOSE_SERIAL_H

#include "GenericPage.h"

#include <QRegExp>

namespace Ui {
    class ChooseSerial;
}

class ChooseSerial : public GenericPage
{
    Q_OBJECT
public:
    explicit ChooseSerial(QWidget *parent = nullptr);
    ~ChooseSerial() override;

    void setValues(const QVariantHash &args) override;
    QVariantHash values() const override;
    bool isValid() const override;

public slots:
    void load();

private:
    Ui::ChooseSerial *ui;
    QRegExp m_rx;
    bool m_isValid = false;
};

#endif
