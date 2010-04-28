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
#include <QThread>
#include <QMutex>
#include <QEventLoop>
#include <QWaitCondition>
#include <KPasswordDialog>

#include "QCups.h"
#include <cups/cups.h>

namespace QCups
{
    class Request : public QObject
    {
        Q_OBJECT
    public slots:
        void request(Result *result, ipp_op_e operation, const QString &resource,
                     Arguments reqValues, bool needResponse);
        void cancelJob(Result *result, const QString &destName, int jobId);
        void cupsAdminSetServerSettings(Result *result, const HashStrStr &userValues);
        void cupsAdminGetServerSettings(Result *result);
        void cupsPrintCommand(Result *result, const QString &name,
                              const QString &command, const QString &title);

    signals:
        void showPasswordDlg(QEventLoop *loop, const QString &username, bool showErrorMessage);
        void finished();

    private:
        bool retry();
    };

    class CupsThreadRequest : public QThread
    {
        Q_OBJECT
    public:
        CupsThreadRequest(QObject *parent = 0);
        ~CupsThreadRequest();

        Request *req;

    private:
        void run();

    };
};

Q_DECLARE_METATYPE(ipp_op_e);
Q_DECLARE_METATYPE(QEventLoop*);
Q_DECLARE_METATYPE(QCups::Arguments);
Q_DECLARE_METATYPE(QCups::ReturnArguments);
Q_DECLARE_METATYPE(QCups::Result*);
Q_DECLARE_METATYPE(QCups::HashStrStr);

#endif
