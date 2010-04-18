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


#include <QApplication>
#include <QPointer>
#include <KPasswordDialog>
#include <KLocale>
#include <QTimer>
#include <KDebug>

Q_DECLARE_METATYPE(QEventLoop*);
using namespace Cups;

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
   inUse(false),
   m_thread(new CupsThreadRequest(this))
{
    m_thread->start();
    while (!m_thread->req) {
        usleep(1);
    }
    connect(m_thread->req, SIGNAL(showPasswordDlg(const QString &, bool)),
            this, SLOT(showPasswordDlg(const QString &, bool)), Qt::BlockingQueuedConnection);
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

void QCups::initialize()
{
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

QList<QCups::Destination> QCups::getDests(int mask, const QStringList &requestedAttr)
{
//     QList<QCups::Destination> ret;
//     QHash<QString, QVariant> request;
//     request["printer-type"] = CUPS_PRINTER_LOCAL;
//     if (mask >= 0) {
//         request["printer-type-mask"] = mask;
//     }
//     request["requested-attributes"] = requestedAttr;
//     kDebug() << request;
//     CupsThreadRequest *thread;
//     thread = new CupsThreadRequest(CUPS_GET_PRINTERS, "/", request);
//     thread->execute();
//     ret = thread->responseValues();
//     return ret;

    Arguments request;
    request["printer-type"] = CUPS_PRINTER_LOCAL;
    if (mask >= 0) {
        request["printer-type-mask"] = mask;
    }
    request["requested-attributes"] = requestedAttr;
kDebug() << "getDests BEGIN" << QThread::currentThreadId();
    QEventLoop *loop = new QEventLoop;

    kDebug() << "getDests BEGIN invoke";
    QMetaObject::invokeMethod(NCups::instance()->m_thread->req,
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(QEventLoop*, loop),
                              Q_ARG(ipp_op_e, CUPS_GET_PRINTERS),
                              Q_ARG(QString, "/"),
                              Q_ARG(Arguments, request));

    loop->exec();
    ReturnArguments ret = loop->property("return").value<ReturnArguments>();
    kDebug() << "getDests END";
    delete loop;
    return ret;
}

QList<QCups::Destination> NCups::getDests(int mask, const QStringList &requestedAttr)
{
    Arguments request;
    request["printer-type"] = CUPS_PRINTER_LOCAL;
    if (mask >= 0) {
        request["printer-type-mask"] = mask;
    }
    request["requested-attributes"] = requestedAttr;
kDebug() << "getDests BEGIN" << QThread::currentThreadId();
    QEventLoop *loop = new QEventLoop;
    while (inUse) {
        kDebug() << "getDests wait TO EXECUTE";
        // the thread is in use wait to not create a dead lock
        connect(this, SIGNAL(finished()), loop, SLOT(quit()));
        loop->exec();
    }

    kDebug() << "getDests BEGIN invoke";
    QMetaObject::invokeMethod(m_thread->req,
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(QEventLoop*, loop),
                              Q_ARG(ipp_op_e, CUPS_GET_PRINTERS),
                              Q_ARG(QString, "/"),
                              Q_ARG(Arguments, request));
    inUse = true;
    loop->exec();
    ReturnArguments ret = loop->property("return").value<ReturnArguments>();
    inUse = false;
    kDebug() << "getDests END";
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    
    delete loop;
    return ret;
}

void NCups::showPasswordDlg(const QString &username, bool showErrorMessage)
{
    kDebug() << QThread::currentThreadId() << sender();
    dlg = new KPasswordDialog(0, KPasswordDialog::ShowUsernameLine);
    dlg->setPrompt(i18n("Enter an username and a password to complete the task"));
    dlg->setModal(true);
    dlg->setUsername(username);
    if (showErrorMessage) {
        dlg->showErrorMessage(QString(), KPasswordDialog::UsernameError);
        dlg->showErrorMessage(i18n("Wrong username or password"), KPasswordDialog::PasswordError);
    }
QEventLoop *loop = new QEventLoop;
QTimer::singleShot(1, dlg, SLOT(show()));
connect(dlg, SIGNAL(finished()), loop, SLOT(quit()));
connect(dlg, SIGNAL(finished()), loop, SLOT(wakeUp()));
loop->exec();
    // show the dialog
//     if (dlg->exec()) {
//         sender()->setProperty("username", dlg->username());
//         sender()->setProperty("password", dlg->password());
//         sender()->setProperty("canceled", false);
//         delete dlg;
//         kDebug()<< "Finish1";
//     } else {
        // the dialog was canceled
        delete dlg;
        sender()->setProperty("canceled", true);
        kDebug()<< "Password Dialog Canceled Finish2";
//     }
}

bool NCups::setShared(const QString &destName, bool isClass, bool shared)
{
    QHash<QString, QVariant> request;
    request["printer-name"] = destName;
    request["printer-is-class"] = isClass;
    request["printer-is-shared"] = shared;
kDebug() << "setShared BEGIN";
    QEventLoop *loop = new QEventLoop;
    while (inUse) {
        kDebug() << "setShared wait TO EXECUTE";
        // the thread is in use wait to not create a dead lock
        connect(this, SIGNAL(finished()), loop, SLOT(quit()));
        loop->exec();
    }
    kDebug() << "setShared BEGIN invoke";
    QMetaObject::invokeMethod(m_thread->req,
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(QEventLoop*, loop),
                              Q_ARG(ipp_op_e, isClass ? CUPS_ADD_MODIFY_CLASS :
                                                        CUPS_ADD_MODIFY_PRINTER),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request));
//      << request;
    inUse = true;
    loop->exec();
    inUse = false;
    kDebug() <<  "setShared END" << loop->property("return").value<ReturnArguments>();
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    delete loop;
    return false;

//     CupsThreadRequest *thread;
//     thread = new CupsThreadRequest(isClass ? CUPS_ADD_MODIFY_CLASS :
//                                              CUPS_ADD_MODIFY_PRINTER,
//                                    "/admin/", request);
//     thread->execute();
//     return !thread->lastError();;
}

bool QCups::Dest::setAttributes(const QString &destName, bool isClass, const QHash<QString, QVariant> &values, const char *filename)
{
    if (values.isEmpty() && !filename) {
        return false;
    }

    RUN_ACTION(cupsAddModifyClassOrPrinter(destName.toUtf8(), isClass, values, filename))
}

bool QCups::Dest::setShared(const QString &destName, bool isClass, bool shared)
{
//     QHash<QString, QVariant> request;
//     request["printer-name"] = destName;
//     request["printer-is-class"] = isClass;
//     request["printer-is-shared"] = shared;
//     kDebug() << request;
//     CupsThreadRequest *thread;
//     thread = new CupsThreadRequest(isClass ? CUPS_ADD_MODIFY_CLASS :
//                                              CUPS_ADD_MODIFY_PRINTER,
//                                    "/admin/", request);
//     thread->execute();
//     return !thread->lastError();;

    QHash<QString, QVariant> request;
    request["printer-name"] = destName;
    request["printer-is-class"] = isClass;
    request["printer-is-shared"] = shared;
    QEventLoop *loop = new QEventLoop;
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
    kDebug() <<  "setShared END" << loop->property("return").value<ReturnArguments>();
    delete loop;
    return false;

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
