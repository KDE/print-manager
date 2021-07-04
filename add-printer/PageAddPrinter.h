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

#ifndef PAGE_ADD_PRINTER_H
#define PAGE_ADD_PRINTER_H

#include "GenericPage.h"

namespace Ui {
    class PageAddPrinter;
}

class PageAddPrinter : public GenericPage
{
    Q_OBJECT
public:
    explicit PageAddPrinter(QWidget *parent = nullptr);
    ~PageAddPrinter() override;

    void setValues(const QVariantHash &args) override;
    QVariantHash values() const override;
    bool canProceed() const override;

    bool finishClicked() override;

public slots:
    void load();

private slots:
    void checkSelected();
    void on_nameLE_textChanged(const QString &text);

private:
    Ui::PageAddPrinter *ui;
};

#endif
