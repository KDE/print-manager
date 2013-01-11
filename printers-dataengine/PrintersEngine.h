/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti                                *
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

#ifndef PRINTERS_ENGINE_H
#define PRINTERS_ENGINE_H
 
#include <Plasma/DataEngine>

#include <KCupsRequest.h>
 
/**
 * This engine provides all printers the current server has.
 *
 */
class KCupsPrinter;
class KCupsRequest;
class PrintersEngine : public Plasma::DataEngine
{
    Q_OBJECT
public:
    // every engine needs a constructor with these arguments
    PrintersEngine(QObject *parent, const QVariantList &args);
    ~PrintersEngine();

    // Get and set all the jobs we have
    virtual void init();

    // Get the Service class which we run operations on
    virtual Plasma::Service* serviceForSource(const QString &source);

private slots:
    void getPrinters();
    void getPrintersFinished();
    void insertUpdatePrinter(const QString &printerName);
    void insertUpdatePrinter(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);
    void insertUpdatePrinterFinished();
    void printerRemoved(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);

private:
    void updatePrinterSource(const KCupsPrinter &printer);

    QStringList m_printerAttributes;
    KCupsRequest *m_printersRequest;
};
 
#endif // PRINT_MANAGER_ENGINE_H
