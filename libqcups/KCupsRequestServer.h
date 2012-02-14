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

#ifndef KCUPSREQUESTSERVER_H
#define KCUPSREQUESTSERVER_H

#include "KCupsRequestInterface.h"

class KCupsPrinter;
class KCupsServer;
class KDE_EXPORT KCupsRequestServer : public KCupsRequestInterface
{
    Q_OBJECT
public:
    explicit KCupsRequestServer();

    QString serverError() const;

public slots:
    /**
     * Get all available PPDs from the givem make
     * @param make the maker of the printer
     */
    void getPPDS(const QString &make = QString());

    /**
     * Get all devices that could be added as a printer
     * This function emits device()
     */
    void getDevices();

    /**
     * Get all available printers
     * @param mask filter the kind of printer that will be emited (-1 to no filter)
     * @param requestedAttr the attibutes to retrieve from cups
     *
     * THIS function can get the default server dest through the
     * "printer-is-default" attribute BUT it does not get user
     * defined default printer, see cupsGetDefault() on www.cups.org for details
     */
    void getPrinters(const QStringList &requestedAttr = QStringList());

    /**
     * Get all jobs
     * TODO we need to see if we authenticate as root to do some taks
     *      the myJobs will return the user's jobs or the root's jobs
     * @param printer which printer you are requiring jobs for (empty = all printers)
     * @param myJobs true if you only want your jobs
     * @param whichJobs which kind jobs should be sent
     */
    void getJobs(const QString &printer, bool myJobs, int whichJobs, const QStringList &requestedAttr = QStringList());

    /**
     * Adds a printer class
     * @param values the values required to add a class
     */
    void addClass(const QHash<QString, QVariant> &values);

    /**
     * Get the CUPS server settings
     * The result will be available at HashStrStr
     */
    void getServerSettings();

    /**
     * Get the CUPS server settings
     * @param userValues the new server settings
     */
    void setServerSettings(const KCupsServer &server);

signals:
    /**
     * Emited when getPrinters is called
     * @param position The position found on the list
     * @param printer The printer found
     */
    void printer(int position, const KCupsPrinter &printer);
    void server(const KCupsServer &server);
    void device(const QString &dev_class,
                const QString &id,
                const QString &info,
                const QString &makeAndModel,
                const QString &uri,
                const QString &location);
};

#endif // KCUPSREQUESTSERVER_H
