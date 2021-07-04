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

#ifndef KCUPSPASSWORDDIALOG_H
#define KCUPSPASSWORDDIALOG_H

#include <QObject>
#include <QWidget>

class KCupsPasswordDialog : public QObject
{
    Q_OBJECT
public:
    explicit KCupsPasswordDialog(QObject *parent = nullptr);
    void setMainWindow(WId mainwindow);
    void setPromptText(const QString &promptText);

public slots:
    void exec(const QString &username, bool wrongPassword);

    bool accepted() const;
    QString username() const;
    QString password() const;

private:
    bool m_accepted;
    WId m_mainwindow;
    QString m_username;
    QString m_password;
    QString m_promptText;
};

#endif // KCUPSPASSWORDDIALOG_H
