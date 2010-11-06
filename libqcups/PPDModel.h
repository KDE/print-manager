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

#ifndef PPD_MODEL_H
#define PPD_MODEL_H

#include <QAbstractListModel>

#include <cups/cups.h>
#include "QCups.h"

class PPDModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(Role)
public:
    typedef enum {
        PPDName = Qt::UserRole,
        PPDMake,
        PPDMakeAndModel
    } Role;

    explicit PPDModel(const QList<QHash<QString, QVariant> > &ppds, QObject *parent = 0);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    void setMake(const QString &make);

private:
    QList<QHash<QString, QVariant> > m_ppds;
    QString m_make;
};

#endif
