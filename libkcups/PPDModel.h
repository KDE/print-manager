/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti
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

#ifndef PPD_MODEL_H
#define PPD_MODEL_H

#include <QStandardItemModel>
#include <QVariantHash>

struct DriverMatch{
    QString ppd;
    QString match;
};
typedef QList<DriverMatch> DriverMatchList;
class PPDModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(Role)
public:
    typedef enum {
        PPDName = Qt::UserRole,
        PPDMake,
        PPDMakeAndModel
    } Role;

    explicit PPDModel(QObject *parent = nullptr);
    void setPPDs(const QList<QVariantHash> &ppds, const DriverMatchList &driverMatch = DriverMatchList());

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void clear();

private:
    QStandardItem* createPPDItem(const QVariantHash &ppd, bool recommended);
    QStandardItem* findCreateMake(const QString &make);

    QList<QVariantHash> m_ppds;
};

Q_DECLARE_METATYPE(DriverMatchList)
Q_DECLARE_METATYPE(DriverMatch)

#endif
