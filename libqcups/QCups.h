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

#ifndef Q_CUPS_H
#define Q_CUPS_H

#include <kdemacros.h>
#include <QStringList>
#include <QEventLoop>
#include <QHash>
#include <QMutex>

#define DEST_IDLE     3
#define DEST_PRINTING 4
#define DEST_STOPED   5

namespace QCups
{
    typedef QHash<QString, QVariant>         Arguments;
    typedef QHash<QString, QString>          HashStrStr;
    typedef QList<Arguments> ReturnArguments;
    typedef QHash<QString, QVariant>         Destination;
    class KDE_EXPORT Result : public QObject
    {
        Q_OBJECT
    public:
        Result(QObject *parent = 0);
        ~Result();
        void waitTillFinished() const;

        bool hasError() const;
        int lastError() const;
        QString lastErrorString() const;
        ReturnArguments result() const;
        HashStrStr hashStrStr() const;

    signals:
        void finished();
        void device(const QString &dev_class,
                    const QString &id,
                    const QString &info,
                    const QString &makeAndModel,
                    const QString &uri,
                    const QString &location);

    protected:
        void setLastError(int error);
        void setLastErrorString(const QString &errorString);
        void setResult(const ReturnArguments &args);
        void setHashStrStr(const HashStrStr &hash);

    private:
        int m_error;
        QString m_errorString;
        ReturnArguments m_args;
        HashStrStr m_hash;
        friend class Request;
    };

    // Dest Methods
    namespace Dest
    {
        KDE_EXPORT Result* setAttributes(const QString &destName, bool isClass, const QHash<QString, QVariant> &values, const char *filename = NULL);

        KDE_EXPORT Result* setShared(const QString &destName, bool isClass, bool shared);
        KDE_EXPORT Result* getAttributes(const QString &destName, bool isClass, const QStringList &requestedAttr);
        KDE_EXPORT Result* printTestPage(const QString &destName, bool isClass);
        KDE_EXPORT Result* printCommand(const QString &destName, const QString &command, const QString &title);
    }

    KDE_EXPORT Result* cancelJob(const QString &name, int job_id);
    KDE_EXPORT Result* holdJob(const QString &name, int job_id);
    KDE_EXPORT Result* releaseJob(const QString &name, int job_id);
    KDE_EXPORT Result* moveJob(const QString &name, int job_id, const QString &dest_name);
    KDE_EXPORT Result* pausePrinter(const QString &name);
    KDE_EXPORT Result* resumePrinter(const QString &name);
    KDE_EXPORT Result* setDefaultPrinter(const QString &name);
    KDE_EXPORT Result* deletePrinter(const QString &name);
    KDE_EXPORT Result* adminSetServerSettings(const QHash<QString, QString> &userValues);
    KDE_EXPORT Result* getPPDS(const QString &make = QString());

    KDE_EXPORT Result* getDevices();
    // THIS function can get the default server dest throught
    // "printer-is-default" attribute BUT it does not get user
    // defined default printer, see cupsGetDefault() on www.cups.org for details
    KDE_EXPORT Result* getDests(int mask, const QStringList &requestedAttr = QStringList());
    KDE_EXPORT Result* getJobs(const QString &destName, bool myJobs, int whichJobs, const QStringList &requestedAttr = QStringList());

    /*
     The result will be in hashStrStr()
    */
    KDE_EXPORT Result* adminGetServerSettings();

    class Request;
    class CupsThreadRequest;
    class NCups : public QObject
    {
        Q_OBJECT
    public:
        static NCups* instance();
        ~NCups();

        Request* request() const;

    public slots:
        void showPasswordDlg(QMutex *mutex, QEventLoop *loop, const QString &username, bool showErrorMessage);

    private:
        NCups(QObject* parent = 0);
        static NCups* m_instance;
        CupsThreadRequest *m_thread;
    };
}

#endif
