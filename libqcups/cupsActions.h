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

#ifndef CUPS_ACTIONS_H
#define CUPS_ACTIONS_H

#include <QHash>
#include <QVariant>

namespace QCups
{
    bool cupsMoveJob(const char *name, int job_id, const char *dest_name);
    bool cupsPauseResumePrinter(const char *name, bool pause);
    bool cupsSetDefaultPrinter(const char *name);
    bool cupsDeletePrinter(const char *name);
    bool cupsHoldReleaseJob(const char *name, int job_id, bool hold);
    bool cupsAddModifyClassOrPrinter(const char *name, bool is_class, const QHash<QString, QVariant> values, const char *filename = NULL);

    QHash<QString, QVariant> cupsGetAttributes(const char *name, bool is_class, const QStringList &requestedAttr);
    QList<QHash<QString, QVariant> > cupsGetDests(int mask, const QStringList &requestedAttr);
};

#endif
