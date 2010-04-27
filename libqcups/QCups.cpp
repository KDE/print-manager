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

#include <QAbstractEventDispatcher>

using namespace QCups;

class NCups : public QObject
{
    Q_OBJECT
public:
    static NCups* instance();
    ~NCups();

public slots:
    void finished();
    void showPasswordDlg(QEventLoop *loop, const QString &username, bool showErrorMessage);

public:
    NCups(QObject* parent = 0);
    static NCups* m_instance;
    CupsThreadRequest *m_thread;
    QList<QEventLoop*> m_events;

    QEventLoop* begin();
    void end(QEventLoop *loop);
};


NCups* NCups::m_instance = 0;
NCups* NCups::instance()
{
    if (!m_instance) {
        m_instance = new NCups(qApp);
    }

    return m_instance;
}

NCups::NCups(QObject* parent)
 : QObject(parent),
   m_thread(new CupsThreadRequest(this))
{
    m_thread->start();
    while (!m_thread->req) {
        usleep(1);
    }
    connect(m_thread->req, SIGNAL(showPasswordDlg(QEventLoop *, const QString &, bool)),
            this, SLOT(showPasswordDlg(QEventLoop *, const QString &, bool)), Qt::QueuedConnection);

    connect(m_thread->req, SIGNAL(finished()),
            this, SLOT(finished()), Qt::QueuedConnection);
}

NCups::~NCups()
{
    m_thread->quit();
    m_thread->wait();
}

#define RUN_ACTION(blurb) \
                password_retries = 0; \
                ipp_status_t ret; \
                do { \
                    ret = blurb; \
                } while (retry(ret)); \
                return !static_cast<bool>(ret); \

static uint password_retries = 0;
bool retry();

bool retry(ipp_status_t lastError)
{
    kDebug() << "cupsLastErrorString()";
//     kDebug()<< cupsLastErrorString();
    if (lastError == IPP_FORBIDDEN ||
        lastError == IPP_NOT_AUTHORIZED ||
        lastError == IPP_NOT_AUTHENTICATED) {
        switch (password_retries) {
        case 0:
            // try to authenticate as the root user
            cupsSetUser("root");
            break;
        case -1:
        case 3:
            // the authentication failed 3 times
            // OR the dialog was canceld (-1)
            // reset to 0 and quit the do-while loop
            password_retries = 0;
            return false;
        }

        // force authentication
        kDebug() << "cupsDoAuthentication" << password_retries;
        if (cupsDoAuthentication(CUPS_HTTP_DEFAULT, "POST", "/") == 0) {
            // tries to do the action again
            // sometimes just trying to be root works
        }

        return true;
    }
    // the action was not forbidden
    return false;
}

bool QCups::moveJob(const QString &name, int job_id, const QString &dest_name)
{
    RUN_ACTION(cupsMoveJob(name.toUtf8(), job_id, dest_name.toUtf8()))
}

bool QCups::pausePrinter(const QString &name)
{
    RUN_ACTION(cupsPauseResumePrinter(name.toUtf8(), true))
}

bool QCups::resumePrinter(const QString &name)
{
    RUN_ACTION(cupsPauseResumePrinter(name.toUtf8(), false))
}

bool QCups::setDefaultPrinter(const QString &name)
{
    RUN_ACTION(cupsSetDefaultPrinter(name.toUtf8()))
}

bool QCups::deletePrinter(const QString &name)
{
    RUN_ACTION(cupsDeletePrinter(name.toUtf8()))
}

bool QCups::cancelJob(const QString &name, int job_id)
{
//     RUN_ACTION(
// TODO put in a thread
   return cupsCancelJob(name.toUtf8(), job_id);
}

bool QCups::holdJob(const QString &name, int job_id)
{
    RUN_ACTION(cupsHoldReleaseJob(name.toUtf8(), job_id, true))
}

bool QCups::releaseJob(const QString &name, int job_id)
{
    RUN_ACTION(cupsHoldReleaseJob(name.toUtf8(), job_id, false))
}

bool QCups::addModifyClassOrPrinter(const QString &name, bool isClass, const QHash<QString, QVariant> values)
{
    RUN_ACTION(cupsAddModifyClassOrPrinter(name.toUtf8(), isClass, values))
}

QHash<QString, QString> QCups::adminGetServerSettings()
{
    return cupsAdminGetServerSettings();
}

bool QCups::adminSetServerSettings(const QHash<QString, QString> &userValues)
{
    RUN_ACTION(cupsAdminSetServerSettings(userValues))
}

QList<QHash<QString, QVariant> > QCups::getPPDS(const QString &make)
{
    return cupsGetPPDS(make);
}

Result QCups::getDests(int mask, const QStringList &requestedAttr)
{
    Arguments request;
    request["printer-type"] = CUPS_PRINTER_LOCAL;
    if (mask >= 0) {
        request["printer-type-mask"] = mask;
    }
    request["requested-attributes"] = requestedAttr;
kDebug() << "getDests BEGIN" << QThread::currentThreadId();
    QEventLoop *loop = NCups::instance()->begin();

    kDebug() << "getDests BEGIN invoke";
    QMetaObject::invokeMethod(NCups::instance()->m_thread->req,
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(QEventLoop*, loop),
                              Q_ARG(ipp_op_e, CUPS_GET_PRINTERS),
                              Q_ARG(QString, "/"),
                              Q_ARG(Arguments, request));

    loop->exec();
    // remove again after finished
    Result result = loop->property("result").value<Result>();
//     ReturnArguments ret = result.result();
    NCups::instance()->end(loop);
    return result;
}

void NCups::showPasswordDlg(QEventLoop *loop, const QString &username, bool showErrorMessage)
{
    // shows a password dialog to the user
    CupsPasswordDialog *dlg = new CupsPasswordDialog(loop, username, showErrorMessage, 0);
    // if we use exec() and a new request creates a QEventLoop this
    // will NEVER return
    dlg->show();
}

QEventLoop* NCups::begin()
{
    // Create a new event loop to not block the UI while the
    // request is not processed
    QEventLoop *loop = new QEventLoop(qApp);
    if (!m_events.isEmpty()) {
        // There are running events
        m_events.append(loop);
        // Execute till the running event loop finishes
        loop->exec();
        m_events.removeOne(loop);
    }

    // Add it again so another loop is blocked
    m_events.append(loop);
    return loop;
}

void NCups::end(QEventLoop *loop)
{
    m_events.removeOne(loop);
    delete loop;
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}

void NCups::finished()
{
    // Runs the last QEventLoop created
    if (!m_events.isEmpty()) {
        m_events.last()->exit();
    }
}

bool QCups::Dest::setAttributes(const QString &destName, bool isClass, const QHash<QString, QVariant> &values, const char *filename)
{
    if (values.isEmpty() && !filename) {
        return false;
    }

    RUN_ACTION(cupsAddModifyClassOrPrinter(destName.toUtf8(), isClass, values, filename))
}

Result QCups::Dest::setShared(const QString &destName, bool isClass, bool shared)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = destName;
    request["printer-is-class"] = isClass;
    request["printer-is-shared"] = shared;

    kDebug() << "setShared BEGIN";
    QEventLoop *loop = NCups::instance()->begin();
    kDebug() << "setShared BEGIN invoke";
    QMetaObject::invokeMethod(NCups::instance()->m_thread->req,
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(QEventLoop*, loop),
                              Q_ARG(ipp_op_e, isClass ? CUPS_ADD_MODIFY_CLASS :
                                                        CUPS_ADD_MODIFY_PRINTER),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request));

    loop->exec();
    kDebug() <<  "setShared END";
    Result result = loop->property("result").value<Result>();
    NCups::instance()->end(loop);
    return result;
}

bool QCups::Dest::printTestPage(const QString &destName, bool isClass)
{
    RUN_ACTION(cupsPrintTestPage(destName.toUtf8(), isClass))
}

bool QCups::Dest::printCommand(const QString &destName, const QString &command, const QString &title)
{
    return cupsPrintCommand(destName.toUtf8(), command.toUtf8(), title.toUtf8());
}

QHash<QString, QVariant> QCups::Dest::getAttributes(const QString &destName, bool isClass, const QStringList &requestedAttr)
{
    return cupsGetAttributes(destName.toUtf8(), isClass, requestedAttr);
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

#include "QCups.moc"
