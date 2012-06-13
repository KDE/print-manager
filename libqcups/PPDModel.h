/***************************************************************************
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

#ifndef PPD_MODEL_H
#define PPD_MODEL_H

#include <QStandardItemModel>
#include <QVariantHash>

struct DriverMatch{
    QString ppd;
    QString match;
};
typedef QList<DriverMatch> DriverMatchList;
class PPDModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(Role)
public:
    typedef enum {
        PPDName = Qt::UserRole,
        PPDMake,
        PPDMakeAndModel
    } Role;

    explicit PPDModel(QObject *parent = 0);
    void setPPDs(const QList<QVariantHash> &ppds, const DriverMatchList &driverMatch = DriverMatchList());

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void clear();

private:
    QStandardItem* createPPDItem(const QVariantHash &ppd, bool recommended);
    QStandardItem* findCreateMake(const QString &make);

    QList<QVariantHash> m_ppds;
};

Q_DECLARE_METATYPE(DriverMatchList)
Q_DECLARE_METATYPE(DriverMatch)

#endif
