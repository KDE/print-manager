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

#ifndef PRINTER_MODEL_H
#define PRINTER_MODEL_H

#include <QStandardItemModel>

#include <cups/cups.h>
#include "QCups.h"

class PrinterModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(JobAction)
    Q_ENUMS(Role)
public:
    typedef enum {
        DestStatus = Qt::UserRole,
        DestName,
        DestIsDefault,
        DestIsShared,
        DestIsClass,
        DestLocation,
        DestDescription,
        DestKind
    } Role;

    typedef enum {
        Cancel,
        Hold,
        Release,
        Move
    } JobAction;

    typedef enum {
        ColStatus = 0,
        ColName,
        ColUser,
        ColCreated,
        ColCompleted,
        ColPrinter
    } Columns;

    explicit PrinterModel(WId parentId, QObject *parent = 0);

    Qt::ItemFlags flags(const QModelIndex &index) const;

public slots:
    void update();

private:
    WId m_parentId;

    int destRow(const QString &destName);
    void insertDest(int pos, const QCups::Destination &dest);
    void updateDest(QStandardItem *item, const QCups::Destination &dest);

    QString destStatus(int state, const QString &message) const;
};

#endif
