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

#include <QPointer>
#include <KPasswordDialog>
#include <KLocale>
#include <KDebug>

using namespace QCups;

#define RUN_ACTION(blurb) \
                password_retries = 0; \
                ipp_status_t ret; \
                do { \
                    ret = blurb; \
                } while (retry(ret)); \
                return !static_cast<bool>(ret); \

static uint password_retries = 0;
bool retry();

const char * my_password_cb(const char *)
{
    kDebug() << "password_retries" << password_retries;
    if (password_retries == 3) {
        // cancel the authentication
        cupsSetUser(NULL);
        return NULL;
    }
    QPointer<KPasswordDialog> dlg = new KPasswordDialog(0, KPasswordDialog::ShowUsernameLine);
    dlg->setPrompt(i18n("Enter an username and a password to complete the task"));
    dlg->setUsername(QString::fromUtf8(cupsUser()));
    // check if the password retries is more than 0 and show an error
    if (password_retries++) {
        dlg->showErrorMessage(QString(), KPasswordDialog::UsernameError);
        dlg->showErrorMessage(i18n("Wrong username or password"), KPasswordDialog::PasswordError);
    }
    dlg->setUsername(QString::fromUtf8(cupsUser()));
    if (dlg->exec()) {
        QString username = dlg->username();
        QString password = dlg->password();
        delete dlg;
        cupsSetUser(username.toUtf8());
        return password.toUtf8();
    }
    delete dlg;
    // the dialog was canceled
    password_retries = -1;
    cupsSetUser(NULL);
    return NULL;
}

bool retry(ipp_status_t lastError)
{
    kDebug() << "cupsLastErrorString()" << cupsLastErrorString();
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
    cupsSetPasswordCB(my_password_cb);
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

QList<Destination> QCups::getDests(int mask, const QStringList &requestedAttr)
{
    return cupsGetDests(mask, requestedAttr);
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
    QHash<QString, QVariant> values;
    values["printer-is-shared"] = shared;
    return setAttributes(destName, isClass, values);
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
