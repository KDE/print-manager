/***************************************************************************
 *   Copyright (C) 2010 by Glauber M. Dantas                               *
 *   glauber.md@gmail.com                                                  *
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef SYSTEM_PREFERENCES_H
#define SYSTEM_PREFERENCES_H

#include <KDialog>

namespace Ui {
    class SystemPreferences;
}
class KCupsServer;
class SystemPreferences : public KDialog
{
    Q_OBJECT
public:
    SystemPreferences(QWidget *parent = 0);
    ~SystemPreferences();

private slots:
    void server(const KCupsServer &server);
    void saveFinished();
    void save();

private:
    Ui::SystemPreferences *ui;
};

#endif
