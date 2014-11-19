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

#ifndef KIPPREQUEST_H
#define KIPPREQUEST_H

#include <KCupsConnection.h>

class KIppRequestPrivate;
class Q_DECL_EXPORT KIppRequest
{
    Q_DECLARE_PRIVATE(KIppRequest)
public:
    KIppRequest();
    KIppRequest(const KIppRequest &other);
    KIppRequest(ipp_op_t operation, const char *resource, const QString &filename = QString());
    ~KIppRequest();

    ipp_op_t operation() const;
    QString resource() const;
    QString filename() const;

    ipp_t *sendIppRequest() const;

    void addString(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QString &value);
    void addStringList(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QStringList &value);
    void addInteger(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, int value);
    void addBoolean(ipp_tag_t group, const QString &name, bool value);
    void addVariantValues(const QVariantHash &values);
    void addPrinterUri(const QString &printerName, bool isClass = false);

    static QString assembleUrif(const QString &name, bool isClass);

    /**
     * Makes a copy of the KIppRequest object other.
     */
    KIppRequest &operator=(const KIppRequest &other);


private:
    KIppRequestPrivate *d_ptr;
};

Q_DECLARE_METATYPE(KIppRequest)

#endif // KIPPREQUEST_H
