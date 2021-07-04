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

#ifndef PRINTER_PAGE_H
#define PRINTER_PAGE_H

#include <QWidget>
#include <KCupsPrinter.h>

class PrinterPage : public QWidget
{
    Q_OBJECT
public:
    explicit PrinterPage(QWidget *parent = nullptr);
    virtual bool hasChanges() { return false; }

public:
    virtual void save() {}
    virtual QVariantHash modifiedValues() const;
    virtual QStringList neededValues() const { return QStringList(); }
    virtual void setRemote(bool remote);

signals:
    void changed(bool hasChanges);
};

#endif
