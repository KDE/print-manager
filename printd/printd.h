/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#ifndef PRINTD_H
#define PRINTD_H

#include <KDEDModule>
#include <QHash>

#include <cups/cups.h>

class QTimer;

class PrintQueueTray;

class PrintD : public KDEDModule
{
Q_OBJECT

public:
    PrintD(QObject *parent, const QList<QVariant>&);
    ~PrintD();

private slots:
    void readConfig();
    void checkJobs();

    void fillMenu();

private:
    QHash<QString, QHash<QString, QString> > destsMessages() const;
    QString m_lastTitle, m_lastSubTitle;
    QHash<QString, int> m_jobsPerPrinter;

    QTimer *m_jobsTimer;
    bool m_onlyMyJobs;

    PrintQueueTray *m_trayIcon;
};

#endif
