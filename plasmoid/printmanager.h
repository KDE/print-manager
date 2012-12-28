/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef PRINTMANAGER_H
#define PRINTMANAGER_H

#include <Plasma/PopupApplet>

namespace Plasma
{
    class DeclarativeWidget;
}

class PrintQueueModel;
class PrintManager : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    PrintManager(QObject *parent, const QVariantList &args);
    ~PrintManager();

    void init();
    QGraphicsWidget *graphicsWidget();

protected Q_SLOTS:
    void toolTipAboutToShow();
    void setActive(bool active = true);

protected:
    void constraintsEvent(Plasma::Constraints constraints);

private:
    Plasma::DeclarativeWidget *m_declarativeWidget;
    PrintQueueModel *m_printQueueModel;
};

K_EXPORT_PLASMA_APPLET(printmanager, PrintManager)

#endif // PRINTMANAGER_H
