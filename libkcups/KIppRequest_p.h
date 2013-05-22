/***************************************************************************
 *   Copyright (C) 2010-2013 by Daniel Nicoletti                           *
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

#ifndef KIPPREQUEST_P_H
#define KIPPREQUEST_P_H

#include <KCupsConnection.h>

class KCupsRawRequest
{
public:
    ipp_tag_t group;
    ipp_tag_t valueTag;
    QString name;
    QVariant value;
};

class KIppRequestPrivate
{
public:
    void addRequest(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QVariant &value);
    void addRawRequestsToIpp(ipp_t *ipp) const;

    ipp_op_t operation;
    QString resource;
    QString filename;
    mutable QList<KCupsRawRequest> rawRequests;
};

static const char **qStringListToCharPtrPtr(const QStringList &list)
{
    QList<QByteArray> qbaList;

    const char **ptr = new const char *[list.size() + 1];
    qbaList.reserve(qbaList.size() + list.size());
    QByteArray qba;
    for (int i = 0; i < list.size(); ++i) {
        qba = list.at(i).toUtf8();
        qbaList.append(qba);
        ptr[i] = qba.constData();
    }
    ptr[list.size()] = 0;
    return ptr;
}

bool rawRequestGroupLessThan(const KCupsRawRequest &a, const KCupsRawRequest &b)
{
     return a.group < b.group;
}

#endif // KIPPREQUEST_P_H
