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

#ifndef KCUPS_REQUEST_H
#define KCUPS_REQUEST_H

#include <QObject>
#include <QEventLoop>

#include "KCupsConnection.h"
#include "KCupsJob.h"
#include "KCupsPrinter.h"
#include "KCupsServer.h"

typedef QList<KCupsPrinter> KCupsPrinters;
typedef QList<KCupsJob> KCupsJobs;

class KDE_EXPORT KCupsRequest : public QObject
{
    Q_OBJECT
public:
    /**
     * Default constructor, it takes no parent
     * because it will move to KCupsConnection thread
     *
     * Before calling any method connect to finished() signal or
     * use waitTillFinished().
     * You must delete the object manually after finished
     * using deleteLater().
     */
    KCupsRequest();

    /**
     * This method creates an event loop
     * and quits after the request is finished
     */
    void waitTillFinished();

    /**
     * This method returns true if there was an error with the request
     */
    bool hasError() const;
    int error() const;
    QString serverError() const;
    QString errorMsg() const;

    /**
     * When renewDBusSubscription() is called the result is stored here
     * @returns -1 when failed
     */
    int subscriptionId() const;

    /**
     * Non empty when getPrinters is called and finish is emitted
     * @param position The position found on the list
     * @param printer The printer found
     */
    KCupsPrinters printers() const;

    /**
     * Non empty when getPPDs is called and finish is emitted
     */
    ReturnArguments ppds() const;

    /**
     * Non empty when getServerSettings() is called and finish is emitted
     */
    KCupsServer serverSettings();

    /**
     * Non empty when getJobs is called and finish is emitted
     * @param position The it will be processed
     * @param job The printer found
     */
    KCupsJobs jobs() const;

    /**
     * Get all available PPDs from the givem make
     * @param make the maker of the printer
     */
    Q_INVOKABLE void getPPDS(const QString &make = QString());

    /**
     * Get all devices that could be added as a printer
     * This method emits device()
     */
    Q_INVOKABLE void getDevices();

    /**
     * Get all available printers
     * @param mask filter the kind of printer that will be emitted (-1 to no filter)
     * @param requestedAttr the attibutes to retrieve from cups
     * This method emits printer()
     *
     * THIS function can get the default server dest through the
     * "printer-is-default" attribute BUT it does not get user
     * defined default printer, see cupsGetDefault() on www.cups.org for details
     */
    Q_INVOKABLE void getPrinters(KCupsPrinter::Attributes attributes, const QVariantHash &arguments = QVariantHash());

    /**
     * Get all available printers
     * @param mask filter the kind of printer that will be emitted (-1 to no filter)
     * @param requestedAttr the attibutes to retrieve from cups
     * This method emits printer()
     *
     * THIS function can get the default server dest through the
     * "printer-is-default" attribute BUT it does not get user
     * defined default printer, see cupsGetDefault() on www.cups.org for details
     */
    Q_INVOKABLE void getPrinters(KCupsPrinter::Attributes attributes, cups_ptype_t mask);

    /**
     * Get attributes from a given printer
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param attributes The attributes you are requesting
     *
     * @return The return will be stored in \sa printers()
     */
    Q_INVOKABLE void getPrinterAttributes(const QString &printerName, bool isClass, KCupsPrinter::Attributes attributes);

    /**
     * Get all jobs
     * This method emits job()
     * TODO we need to see if we authenticate as root to do some taks
     *      the myJobs will return the user's jobs or the root's jobs
     * @param printer which printer you are requiring jobs for (empty = all printers)
     * @param myJobs true if you only want your jobs
     * @param whichJobs which kind jobs should be sent
     */
    Q_INVOKABLE void getJobs(const QString &printerName, bool myJobs, int whichJobs, KCupsJob::Attributes attributes);

    /**
     * Get attributes from a given printer
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param attributes The attributes you are requesting
     *
     * @return The return will be stored in \sa printers()
     */
    Q_INVOKABLE void getJobAttributes(int jobId, const QString &printerUri, KCupsJob::Attributes attributes);

    /**
     * This makes KCupsConnection emit signals to the following
     * events:
     * "job-created"
     * "job-completed"
     * "job-state-changed"
     * "job-state"
     * "printer-added"
     * "printer-deleted"
     * "printer-state-changed"
     */
    Q_INVOKABLE void createDBusSubscription(const QStringList &events);

    Q_INVOKABLE void cancelDBusSubscription(int subscriptionId);

    /**
     * Adds a printer class
     * @param values the values required to add a class
     */
    void addClass(const QVariantHash &values);

    /**
     * Get the CUPS server settings
     * This method emits server()
     */
    Q_INVOKABLE void getServerSettings();

    /**
     * Get the CUPS server settings
     * @param userValues the new server settings
     */
    Q_INVOKABLE void setServerSettings(const KCupsServer &server);

    // ---- Printer Methods
    /**
     * Set the Printer attributes
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param attributes The new attributes of the printer
     * @param filename The file name in case of changing the PPD
     */
    void setAttributes(const QString &printerName,
                       bool isClass,
                       const QVariantHash &attributes,
                       const QString &filename = QString());

    /**
     * Set if a given printer should be shared among other cups
     * @param printer The printer to apply the change
     * @param isClass True it is a printer class
     * @param shared True if it should be shared
     */
    void setShared(const QString &printerName, bool isClass, bool shared);

    /**
     * Set if a given printer should be the default one among others
     * @param printer The printer to apply the change
     */
    void setDefaultPrinter(const QString &printerName);

    /**
     * Pause the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void pausePrinter(const QString &printerName);

    /**
     * Resume the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void resumePrinter(const QString &printerName);

    /**
     * Allows the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void acceptJobs(const QString &printerName);

    /**
     * Prevents the given printer from receiving jobs
     * @param printer The printer to apply the change
     */
    void rejectJobs(const QString &printerName);

    /**
     * Delete the given printer, if it's not local it's not
     * possible to delete it
     * @param printer The printer to apply the change
     */
    void deletePrinter(const QString &printerName);

    /**
     * Print a test page
     * @param printerName The printer where the test should be done
     * @param isClass True it is a printer class
     */
    void printTestPage(const QString &printerName, bool isClass);

    /**
     * Print a command test
     * @param printerName The printer where the test should be done
     * @param command The command to print
     * @param title The title of the command
     */
    Q_INVOKABLE void printCommand(const QString &printerName, const QString &command, const QString &title);

    // Jobs methods
    /**
     * Cancels tries to cancel a given job
     * @param printerName the destination name (printer)
     * @param jobId the job identification
     */
    void cancelJob(const QString &printerName, int jobId);

    /**
     * Holds the printing of a given job
     * @param printerName the destination name (printer)
     * @param jobId the job identification
     */
    void holdJob(const QString &printerName, int jobId);

    /**
     * Holds the printing of a given job
     * @param printerName the destination name (printer)
     * @param jobId the job identification
     */
    void releaseJob(const QString &printerName, int jobId);

    /**
     * Restart the printing of a given job
     * @param printerName the destination name (printer)
     * @param jobId the job identification
     */
    void restartJob(const QString &printerName, int jobId);

    /**
     * Holds the printing of a given job
     * @param fromDestName the destination name which holds the job
     * @param jobId the job identification
     * @param toDestName the destination to hold the job
     */
    void moveJob(const QString &fromPrinterName, int jobId, const QString &toPrinterName);

signals:
    void device(const QString &dev_class,
                const QString &id,
                const QString &info,
                const QString &makeAndModel,
                const QString &uri,
                const QString &location);

    void finished();

private:
    void invokeMethod(const char *method,
                      const QVariant &arg1 = QVariant(),
                      const QVariant &arg2 = QVariant(),
                      const QVariant &arg3 = QVariant(),
                      const QVariant &arg4 = QVariant(),
                      const QVariant &arg5 = QVariant(),
                      const QVariant &arg6 = QVariant(),
                      const QVariant &arg7 = QVariant(),
                      const QVariant &arg8 = QVariant());
    Q_INVOKABLE void doOperation(int operation, const QString &resource, const QVariantHash &request);
    void setError(int error, const QString &errorMsg);
    void setFinished();

    QEventLoop m_loop;
    bool m_finished;
    int m_error;
    QString m_errorMsg;
    ReturnArguments m_ppds;
    KCupsServer m_server;
    int m_subscriptionId;
    KCupsPrinters m_printers;
    KCupsJobs m_jobs;
};

#endif // KCUPS_REQUEST_H
