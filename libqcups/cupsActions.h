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

#include <cups/cups.h>

namespace QCups
{
    ipp_status_t cupsMoveJob(const char *name, int job_id, const char *dest_name);
    ipp_status_t cupsPauseResumePrinter(const char *name, bool pause);
    ipp_status_t cupsSetDefaultPrinter(const char *name);
    ipp_status_t cupsDeletePrinter(const char *name);
    ipp_status_t cupsHoldReleaseJob(const char *name, int job_id, bool hold);
    ipp_status_t cupsAddModifyClassOrPrinter(const char *name, bool is_class, const QHash<QString, QVariant> values, const char *filename = NULL);
    ipp_status_t cupsPrintTestPage(const char *name, bool is_class);
    bool cupsPrintCommand(const char *name, const char *command, const char *title);
    ipp_status_t cupsAdminSetServerSettings(const QHash<QString, QString> &userValues);

    QList<QHash<QString, QVariant> > cupsGetPPDS(const QString &make);
    QHash<QString, QVariant> cupsGetAttributes(const char *name, bool is_class, const QStringList &requestedAttr);
    QList<QHash<QString, QVariant> > cupsGetDests(int mask, const QStringList &requestedAttr);
    QHash<QString, QString> cupsAdminGetServerSettings();

    class CupsThread : public QThread
    {
        Q_OBJECT
    public:
        CupsThread(ipp_t *request, const char *resource);
        ~CupsThread();

        ipp_t* execute();
        ipp_t* response() const;
        ipp_status_t lastError() const;
//         QString lastErrorString() const;
    private:
        void run();
        QMutex mutex;
        ipp_t *m_request, *m_response;
        ipp_status_t m_lastError;

        const char *m_resource;
    };
};

#endif
