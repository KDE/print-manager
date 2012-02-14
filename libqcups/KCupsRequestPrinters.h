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

#ifndef KCUPSREQUESTPRINTERS_H
#define KCUPSREQUESTPRINTERS_H

#include "KCupsRequestInterface.h"
#include "KCupsConnection.h"

class KDE_EXPORT KCupsRequestPrinters : public KCupsRequestInterface
{
    Q_OBJECT
public:
    explicit KCupsRequestPrinters();

public slots:
    /**
     * Set the Printer attributes
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param attributes The new attributes of the printer
     * @param filename The file name in case of changing the PPD
     */
    void setAttributes(const QString &printer, bool isClass, const Arguments &attributes, const char *filename = NULL);

    /**
     * Set if a given printer should be shared among other cups
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param shared True if it should be shared
     */
    void setShared(const QString &printer, bool isClass, bool shared);

    /**
     * Set if a given printer should be the default one among others
     * @param printer The printer to apply the change
     */
    void setDefaultPrinter(const QString &name);

    /**
     * Pause the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void pausePrinter(const QString &name);

    /**
     * Resume the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void resumePrinter(const QString &name);

    /**
     * Delete the given printer, if it's not local it's not
     * possible to delete it
     * @param printer The printer to apply the change
     */
    void deletePrinter(const QString &name);

    /**
     * Get attributes from a given printer
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param requestedAttr The attributes you are requesting
     */
    void getAttributes(const QString &printer, bool isClass, const QStringList &requestedAttr);

    /**
     * Print a test page
     * @param printer The printer where the test should be done
     * @param isClass True it is a printer class
     */
    void printTestPage(const QString &printer, bool isClass);

    /**
     * Print a command test
     * @param printer The printer where the test should be done
     * @param command The command to print
     * @param title The title of the command
     */
    void printCommand(const QString &printer, const QString &command, const QString &title);
};

#endif // KCUPSREQUESTPRINTERS_H
