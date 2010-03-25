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
#include <cups/cups.h>

#include <QPointer>
#include <KPasswordDialog>
#include <KLocale>
#include <KDebug>

using namespace QCups;

#define RUN_ACTION(blurb) \
                password_retries = 0; \
                do { \
                    blurb; \
                } while (retry()); \
                return !cupsLastError(); \

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

bool retry()
{
//     kDebug() << "cupsLastErrorString()" << cupsLastErrorString();
    if (cupsLastError() == IPP_FORBIDDEN ||
        cupsLastError() == IPP_NOT_AUTHORIZED ||
        cupsLastError() == IPP_NOT_AUTHENTICATED) {
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
    RUN_ACTION(cupsCancelJob(name.toUtf8(), job_id))
}

bool QCups::holdJob(const QString &name, int job_id)
{
    RUN_ACTION(cupsHoldReleaseJob(name.toUtf8(), job_id, true))
}

bool QCups::releaseJob(const QString &name, int job_id)
{
    RUN_ACTION(cupsHoldReleaseJob(name.toUtf8(), job_id, false))
}

bool QCups::addModifyPrinter(const QString &name, const QHash<QString, QVariant> values)
{
    RUN_ACTION(cupsAddModifyPrinter(name.toUtf8(), values))
}

Printer::Printer(QObject *parent)
  : QObject(parent)
{
}

Printer::Printer(const QString &destName, QObject *parent)
  : QObject(parent)
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(destName.toUtf8(), NULL, num_dests, dests);
    if (dest == NULL) {
        return;
    }
    m_destName = destName;

    // store the printer values in a hash
    for (int i = 0; i < dest->num_options; i++) {
        m_values[dest->options[i].name] = QString::fromUtf8(dest->options[i].value);
    }

  kDebug() << m_values;
/*
    // store if the printer is shared
    value = cupsGetOption("printer-is-shared", dest->num_options, dest->options);
    if (value) {
        // Here we have a cups docs bug where the SHARED returned
        // value is the string "true" or "false", and not '1' or '0'
        m_shared = value[0] == 't' || value[0] == '1';
    }

    // store the printer location
    if (value = cupsGetOption("printer-location", dest->num_options, dest->options)) {
        m_location = QString::fromUtf8(value);
    }

    // store the printer description
    if (value = cupsGetOption("printer-info", dest->num_options, dest->options)) {
        m_description = QString::fromUtf8(value);
    }

    // store the printer kind
    if (value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options)) {
        m_makeAndModel = QString::fromUtf8(value);
    }

    // store the printer uri
    if (value = cupsGetOption("device-uri", dest->num_options, dest->options)) {
        m_connection = QString::fromUtf8(value);
    }

    cupsFreeDests(num_dests, dests);*/
}

QString Printer::value(const QString &name) const
{
    if (m_values.contains(name)) {
        return m_values[name];
    }
    return QString();
}

bool Printer::save(QHash<QString, QVariant> values)
{
    if (values.isEmpty()) {
        return false;
    }

    RUN_ACTION(cupsAddModifyPrinter(m_destName.toUtf8(), values))
}

bool Printer::setShared(const QString &destName, bool shared)
{
    QHash<QString, QVariant> values;
    values["printer-is-shared"] = shared;
    RUN_ACTION(cupsAddModifyPrinter(destName.toUtf8(), values))
}

QHash<QString, QVariant> Printer::getAttributes(const QString &destName, const QStringList &requestedAttr)
{
    return cupsGetAttributes(destName.toUtf8(), requestedAttr);
}

#include "QCups.moc"
