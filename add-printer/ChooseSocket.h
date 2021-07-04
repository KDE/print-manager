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

#ifndef CHOOSE_SOCKET_H
#define CHOOSE_SOCKET_H

#include "GenericPage.h"

namespace Ui {
    class ChooseSocket;
}
class ChooseSocket : public GenericPage
{
    Q_OBJECT
public:
    explicit ChooseSocket(QWidget *parent = nullptr);
    ~ChooseSocket() override;

    void setValues(const QVariantHash &args) override;
    QVariantHash values() const override;
    bool isValid() const override;
    bool canProceed() const override;

private slots:
    void on_addressLE_textChanged(const QString &text);

private:
    Ui::ChooseSocket *ui;
    bool m_isValid = false;
};

#endif
