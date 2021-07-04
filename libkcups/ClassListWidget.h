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

#ifndef CLASS_LIST_WIDGET_H
#define CLASS_LIST_WIDGET_H

#include <QStandardItemModel>
#include <QListView>
#include <QTimer>

#include <KPixmapSequenceOverlayPainter>

class KCupsRequest;
class Q_DECL_EXPORT ClassListWidget : public QListView
{
    Q_OBJECT
    Q_PROPERTY(QString selectedPrinters READ selectedPrinters WRITE setSelectedPrinters USER true)
    Q_PROPERTY(bool showClasses READ showClasses WRITE setShowClasses)
public:
    explicit ClassListWidget(QWidget *parent = nullptr);
    ~ClassListWidget();

    bool hasChanges();
    void setPrinter(const QString &printer);
    QString selectedPrinters() const;
    void setSelectedPrinters(const QString &selected);
    bool showClasses() const;
    void setShowClasses(bool enable);
    QStringList currentSelected(bool uri) const;

signals:
    void changed(bool changed);
    void changed(const QString &selected);

private slots:
    void init();
    void loadFinished(KCupsRequest *request);
    void modelChanged();

private:
    void updateItemState(QStandardItem *item) const;

    QString m_printerName;
    QStringList m_selectedPrinters;
    KPixmapSequenceOverlayPainter *m_busySeq;
    KCupsRequest *m_request = nullptr;
    bool m_changed;
    bool m_showClasses = false;
    QStandardItemModel *m_model;
    QTimer m_delayedInit;
};

#endif
