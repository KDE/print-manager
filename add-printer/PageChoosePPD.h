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

#ifndef PAGE_CHOOSE_PPD_H
#define PAGE_CHOOSE_PPD_H

#include "GenericPage.h"

#include <QStackedLayout>

#include <KIO/Job>

namespace Ui {
    class PageChoosePPD;
}

class SelectMakeModel;
class PageChoosePPD : public GenericPage
{
    Q_OBJECT
public:
    explicit PageChoosePPD(const QVariantHash &args = QVariantHash(), QWidget *parent = nullptr);
    ~PageChoosePPD() override;

    bool canProceed() const override;
    void setValues(const QVariantHash &args) override;
    QVariantHash values() const override;
    bool isValid() const override;

private slots:
    void checkSelected();
    void selectDefault();
    void resultJob(KJob *job);

private:
    void removeTempPPD();

    Ui::PageChoosePPD *ui;
    bool m_isValid = false;
    SelectMakeModel *m_selectMM;
    QStackedLayout *m_layout;
    QString m_ppdFile;
};

#endif
