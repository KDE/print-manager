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

#ifndef GENERIC_PAGE_H
#define GENERIC_PAGE_H

#include <QWidget>
#include <QHash>
#include <QVariant>

#define ADDING_PRINTER         QLatin1String("add-new-printer")
#define PPD_NAME               QLatin1String("ppd-name")
#define FILENAME               QLatin1String("filename")

class GenericPage : public QWidget
{
    Q_OBJECT
public:
    explicit GenericPage(QWidget *parent = nullptr);
    virtual bool canProceed() const { return true; }
    virtual bool isValid() const { return true; }
    virtual bool isWorking() const { return m_working; }
    virtual void setValues(const QVariantHash &args);
    virtual QVariantHash values() const;

    virtual bool finishClicked() { return false; }

signals:
    void allowProceed(bool allow);
    void proceed();
    void startWorking();
    void stopWorking();

protected slots:
    void working();
    void notWorking();

protected:
    QVariantHash m_args;
    int m_working;
};

#endif
