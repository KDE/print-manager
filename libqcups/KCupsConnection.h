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

#ifndef KCUPSCONNECTION_H
#define KCUPSCONNECTION_H

#include <QThread>

#include <KPasswordDialog>

#include <cups/cups.h>

typedef QList<QVariantHash> ReturnArguments;
class KCupsConnection : public QThread
{
    Q_OBJECT
public:
    /**
     * This is the main Cups class @author Daniel Nicoletti <dantti12@gmail.com>
     *
     * By calling KCupsConnection::global() you have access to it.
     * Due to cups archtecture, this class has to live on a
     * separate thread so we avoid blocking the user interface when
     * the cups call blocks.
     *
     * It is IMPORTANT that we do not create several thread
     * for each cups request, doing so is a valid but breaks our
     * authentication. We could tho store the user information an
     * set the user/password every time it was needed. But I am not
     * sure this is safe.
     *
     * Extending this means either adding methods to the KCupsRequest
     * class which will move to this thread and then run.
     */
    static KCupsConnection* global();
    ~KCupsConnection();

protected:
    friend class KCupsRequest;

    virtual void run();
    static bool readyToStart();
    static bool retryIfForbidden();

    static ReturnArguments request(ipp_op_e operation,
                                   const QString &resource,
                                   const QVariantHash &reqValues,
                                   bool needResponse);
private:
    KCupsConnection(QObject *parent = 0);
    static void requestAddValues(ipp_t *request, const QVariantHash &values);
    static ReturnArguments parseIPPVars(ipp_t *response,
                                        int group_tag,
                                        bool needDestName);
    static ipp_t* ippNewDefaultRequest(const QString &name, bool isClass, ipp_op_t operation);
    static QVariant ippAttrToVariant(ipp_attribute_t *attr);

    static KCupsConnection* m_instance;

    bool m_inited;
    KPasswordDialog *m_passwordDialog;
};

#endif // KCUPSCONNECTION_H
