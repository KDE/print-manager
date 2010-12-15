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

#include "QCups.h"

#include "cupsActions.h"
#include "CupsPasswordDialog.h"

#include <QApplication>
#include <QPointer>
#include <KPasswordDialog>
#include <KLocale>
#include <QTimer>
#include <KDebug>

#define CUPS_DATADIR    "/usr/share/cups"

using namespace QCups;

QCupsConnection* QCupsConnection::m_instance = 0;
QCupsConnection* QCupsConnection::instance()
{
    if (!m_instance) {
        m_instance = new QCupsConnection(qApp);
    }

    return m_instance;
}

QCupsConnection::QCupsConnection(QObject* parent)
 : QObject(parent),
   m_thread(new CupsThreadRequest)
{
    m_thread->moveToThread(m_thread);
    m_thread->start();
    while (!m_thread->isRunning()) {
        usleep(1);
    }
}

QCupsConnection::~QCupsConnection()
{
    m_thread->quit();
    m_thread->wait();
}

CupsThreadRequest* QCupsConnection::request() const
{
    return m_thread;
}

Result* QCups::pausePrinter(const QString &name)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = name;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, IPP_PAUSE_PRINTER),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::resumePrinter(const QString &name)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = name;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, IPP_RESUME_PRINTER),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::setDefaultPrinter(const QString &name)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = name;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, CUPS_SET_DEFAULT),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::deletePrinter(const QString &name)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = name;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, CUPS_DELETE_PRINTER),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::cancelJob(const QString &name, int job_id)
{
    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "cancelJob",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(QString, name),
                              Q_ARG(int,     job_id));

    return result;
}

Result* QCups::holdJob(const QString &name, int job_id)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = name;
    request["job-id"] = job_id;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, IPP_HOLD_JOB),
                              Q_ARG(QString, "/jobs/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::releaseJob(const QString &name, int job_id)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = name;
    request["job-id"] = job_id;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, IPP_RELEASE_JOB),
                              Q_ARG(QString, "/jobs/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::moveJob(const QString &name, int job_id, const QString &dest_name)
{
    if (job_id < -1 || name.isEmpty() || dest_name.isEmpty() || job_id == 0) {
        qWarning() << "Internal error, invalid input data" << job_id << name << dest_name;
        return 0;
    }
    QHash<QString, QVariant> request;
    request["printer-name"] = name;
    request["job-id"] = job_id;
    request["job-printer-uri"] = dest_name;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, CUPS_MOVE_JOB),
                              Q_ARG(QString, "/jobs/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::Dest::setAttributes(const QString &destName, bool isClass, const QHash<QString, QVariant> &values, const char *filename)
{
    if (values.isEmpty() && !filename) {
        return 0;
    }

    QHash<QString, QVariant> request(values);
    request["printer-name"] = destName;
    request["printer-is-class"] = isClass;
    if (filename) {
        request["filename"] = filename;
    }

    ipp_op_e op;
    // TODO this seems weird now.. review this code..
    if (isClass && values.contains("member-uris")) {
        op = CUPS_ADD_CLASS;
    } else {
        op = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;
    }

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, op),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::adminGetServerSettings()
{
    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "cupsAdminGetServerSettings",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result));

    return result;
}

Result* QCups::adminSetServerSettings(const QHash<QString, QString> &userValues)
{
    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "cupsAdminSetServerSettings",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(HashStrStr, userValues));

    return result;
}

Result* QCups::getPPDS(const QString &make)
{
    Arguments request;
    if (!make.isEmpty()){
        request["ppd-make-and-model"] = make;
    }
    request["need-dest-name"] = false;
    Result *result = new Result(QCupsConnection::instance());

    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, CUPS_GET_PPDS),
                              Q_ARG(QString, "/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, true));

    return result;
}

Result* QCups::getDevices()
{
    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "cupsGetDevices",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result));

    return result;
}

Result* QCups::getDests(int mask, const QStringList &requestedAttr)
{
    Arguments request;
    request["printer-type"] = CUPS_PRINTER_LOCAL;
    if (mask >= 0) {
        request["printer-type-mask"] = mask;
    }
    request["requested-attributes"] = requestedAttr;
    request["need-dest-name"] = true;
    Result *result = new Result(QCupsConnection::instance());
    result->setProperty("methodName", "getDests");

    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, CUPS_GET_PRINTERS),
                              Q_ARG(QString, "/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, true));

    return result;
}

Result* QCups::getJobs(const QString &destName,
                       bool myJobs,
                       int whichJobs,
                       const QStringList &requestedAttr)
{
    Arguments request;
    if (destName.isEmpty()) {
        request["printer-uri"] = destName;
    } else {
        request["printer-name"] = destName;
    }

    if (myJobs) {
        request["my-jobs"] = myJobs;
    }

    if (whichJobs == CUPS_WHICHJOBS_COMPLETED) {
        request["which-jobs"] = "completed";
    } else if (whichJobs == CUPS_WHICHJOBS_ALL) {
        request["which-jobs"] = "all";
    }

    if (!requestedAttr.isEmpty()) {
        request["requested-attributes"] = requestedAttr;
    }
    request["group-tag-qt"] = IPP_TAG_JOB;

    Result *result = new Result(QCupsConnection::instance());
    result->setProperty("methodName", "getJobs");
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, IPP_GET_JOBS),
                              Q_ARG(QString, "/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, true));

    return result;
}

void QCupsConnection::showPasswordDlg(QMutex *mutex, QEventLoop *loop, const QString &username, bool showErrorMessage)
{
    kDebug() << "---------LOCK";
    mutex->lock();
    kDebug() << "---------LOCK2";
    // shows a password dialog to the user
    CupsPasswordDialog *dlg = new CupsPasswordDialog(mutex, loop, username, showErrorMessage, 0);
    // if we use exec() and a new request creates a QEventLoop this
    // will NEVER return
    dlg->show();
}

Result* QCups::Dest::setShared(const QString &destName, bool isClass, bool shared)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = destName;
    request["printer-is-class"] = isClass;
    request["printer-is-shared"] = shared;
    request["need-dest-name"] = true;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, isClass ? CUPS_ADD_MODIFY_CLASS :
                                                        CUPS_ADD_MODIFY_PRINTER),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::Dest::printTestPage(const QString &destName, bool isClass)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = destName;
    request["printer-is-class"] = isClass;
    request["job-name"] = i18n("Test Page");
    char          resource[1024], /* POST resource path */
                  filename[1024]; /* Test page filename */
    const char    *datadir;       /* CUPS_DATADIR env var */

    /*
     * Locate the test page file...
     */
    datadir = qgetenv("CUPS_DATADIR").isEmpty() ? CUPS_DATADIR : qgetenv("CUPS_DATADIR") ;
    snprintf(filename, sizeof(filename), "%s/data/testprint", datadir);
    request["filename"] = filename;

    /*
     * Point to the printer/class...
     */
    snprintf(resource, sizeof(resource),
             isClass ? "/classes/%s" : "/printers/%s", destName.toUtf8().data());

    Result *result = new Result(QCupsConnection::instance());
    result->setProperty("methodName", "printTestPage");

    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*,   result),
                              Q_ARG(ipp_op_e,  IPP_PRINT_JOB),
                              Q_ARG(QString,   resource),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::Dest::printCommand(const QString &destName, const QString &command, const QString &title)
{
    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "cupsPrintCommand",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(QString, destName),
                              Q_ARG(QString, command),
                              Q_ARG(QString, title));

    return result;
}

Result* QCups::addClass(const QHash<QString, QVariant> &values)
{
    if (values.isEmpty()) {
        return 0;
    }

    QHash<QString, QVariant> request(values);
    request["printer-is-class"] = true;
    request["printer-is-accepting-jobs"] = true;
    request["printer-state"] = IPP_PRINTER_IDLE;

    ipp_op_e op = CUPS_ADD_CLASS;;
//     if (isClass && values.contains("member-uris")) {
//         op = CUPS_ADD_CLASS;
//     } else {
//         op = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;
//     }

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, op),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

Result* QCups::Dest::getAttributes(const QString &destName, bool isClass, const QStringList &requestedAttr)
{
    Arguments request;
    request["printer-name"] = destName;
    request["printer-is-class"] = isClass;
    request["need-dest-name"] = false; // we don't need a dest name since it's a single list
    request["requested-attributes"] = requestedAttr;

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, IPP_GET_PRINTER_ATTRIBUTES),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, true));

    return result;
}

Result::Result(QObject *parent)
 : QObject(parent),
   m_error(0)
{
    // Do not connect the finished() signal to
    // deleteLater() slot, as of sometimes it got
    // deleted before we could use it
}

Result::~Result()
{
}

void Result::waitTillFinished() const
{
    QEventLoop loop;
    connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}

bool Result::hasError() const
{
    return m_error;
}

int Result::lastError() const
{
    return m_error;
}

void Result::setLastError(int error)
{
    m_error = error;
}

QString Result::lastErrorString() const
{
    return m_errorString;
}

void Result::setLastErrorString(const QString &errorString)
{
    m_errorString = errorString;
}

ReturnArguments Result::result() const
{
    return m_args;
}

void Result::setResult(const ReturnArguments &args)
{
    m_args = args;
}

HashStrStr Result::hashStrStr() const
{
    return m_hash;
}

void Result::setHashStrStr(const HashStrStr &hash)
{
    m_hash = hash;
}

KIcon QCups::Dest::icon(const QString &destName, int printerType)
{
    // TODO get the ppd or something to get the real printer icon
    Q_UNUSED(destName)

    if (!(printerType & CUPS_PRINTER_COLOR)) {
        // If the printer is not color it is probably a laser one
        return KIcon("printer-laser");
    } else if (printerType & CUPS_PRINTER_SCANNER) {
        return KIcon("scanner");
    } else {
        return KIcon("printer");
    }
}

#include "QCups.moc"
