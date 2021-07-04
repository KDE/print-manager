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

#ifndef MODIFY_PRINTER_H
#define MODIFY_PRINTER_H

#include "PrinterPage.h"

#include "KCupsRequest.h"

#include <QWidget>

namespace Ui {
    class ModifyPrinter;
}

class ModifyPrinter : public PrinterPage
{
    Q_OBJECT
    Q_ENUMS(Role)
public:
    typedef enum {
        PPDDefault,
        PPDCustom,
        PPDFile,
        PPDList,
        PPDName = Qt::UserRole + 1
    } Role;
    explicit ModifyPrinter(const QString &destName, bool isClass, QWidget *parent = nullptr);
    ~ModifyPrinter() override;

    bool hasChanges() override;
    QVariantHash modifiedValues() const override;
    QStringList neededValues() const override;
    void setRemote(bool remote) override;

    void setValues(const KCupsPrinter &printer);
    void setCurrentMake(const QString &make);
    void setCurrentMakeAndModel(const QString &makeAndModel);

    void save() override;

signals:
    void ppdChanged();

private slots:
    void textChanged(const QString &text);
    void on_makeCB_activated(int index);
    void ppdSelectionAccepted();
    void ppdSelectionRejected();
    void modelChanged();

private:
    Ui::ModifyPrinter *ui;
    QString m_destName, m_make, m_makeAndModel;
    bool m_isClass;
    QVariantHash m_changedValues;
    int m_changes = 0;
};

#endif
