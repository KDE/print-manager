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
    KPasswordDialog dlg(0, KPasswordDialog::ShowUsernameLine);
    dlg.setPrompt(i18n("Enter an username and a password to complete the task"));
    dlg.setUsername(QString::fromLocal8Bit(cupsUser()));
    // check if the password retries is more than 0 and show an error
    if (password_retries++) {
        dlg.showErrorMessage(QString(), KPasswordDialog::UsernameError);
        dlg.showErrorMessage(i18n("Wrong username or password"), KPasswordDialog::PasswordError);
    }
    dlg.setUsername(QString::fromLocal8Bit(cupsUser()));
    if (dlg.exec()) {
        cupsSetUser(dlg.username().toLocal8Bit().data());
        return dlg.password().toLocal8Bit().data();
    }
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
    RUN_ACTION(cupsMoveJob(name.toLocal8Bit().data(), job_id, dest_name.toLocal8Bit().data()))
}

bool QCups::pausePrinter(const QString &name)
{
    RUN_ACTION(cupsPauseResumePrinter(name.toLocal8Bit().data(), true))
}

bool QCups::resumePrinter(const QString &name)
{
    RUN_ACTION(cupsPauseResumePrinter(name.toLocal8Bit().data(), false))
}

bool QCups::setDefaultPrinter(const QString &name)
{
    RUN_ACTION(cupsSetDefaultPrinter(name.toLocal8Bit().data()))
}

bool QCups::deletePrinter(const QString &name)
{
    RUN_ACTION(cupsDeletePrinter(name.toLocal8Bit().data()))
}

bool QCups::cancelJob(const QString &name, int job_id)
{
    RUN_ACTION(cupsCancelJob(name.toLocal8Bit().data(), job_id))
}

bool QCups::holdJob(const QString &name, int job_id)
{
    RUN_ACTION(cupsHoldReleaseJob(name.toLocal8Bit().data(), job_id, true))
}

bool QCups::releaseJob(const QString &name, int job_id)
{
    RUN_ACTION(cupsHoldReleaseJob(name.toLocal8Bit().data(), job_id, false))
}

bool QCups::addModifyPrinter(const QString &name, const QHash<QString, QVariant> values)
{
    RUN_ACTION(cupsAddModifyPrinter(name.toLocal8Bit().data(), values))
}

Printer::Printer(QObject *parent)
  : QObject(parent)
{
}

Printer::Printer(const QString &destName, QObject *parent)
  : QObject(parent)
{
    cups_dest_t *dests;
    const char *value;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(destName.toLocal8Bit(), NULL, num_dests, dests);
    if (dest == NULL) {
        return;
    }
    m_destName = destName;

    // store if the printer is shared
    value = cupsGetOption("printer-is-shared", dest->num_options, dest->options);
    if (value) {
        // Here we have a cups docs bug where the SHARED returned
        // value is the string "true" or "false", and not '1' or '0'
        m_shared = value[0] == 't' || value[0] == '1';
    }

    // store the printer location
    if (value = cupsGetOption("printer-location", dest->num_options, dest->options)) {
        m_location = QString::fromLocal8Bit(value);
    }

    // store the printer description
    if (value = cupsGetOption("printer-info", dest->num_options, dest->options)) {
        m_description = QString::fromLocal8Bit(value);
    }

    // store the printer kind
    if (value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options)) {
        m_makeAndModel = QString::fromLocal8Bit(value);
    }

    // store the printer uri
    if (value = cupsGetOption("device-uri", dest->num_options, dest->options)) {
        m_connection = QString::fromLocal8Bit(value);
    }

    cupsFreeDests(num_dests, dests);
}

void Printer::setDescription(const QString &description)
{
    if (m_description != description) {
        m_values.remove("printer-info");
    } else {
        m_values["printer-info"] = description;
    }
}

void Printer::setLocation(const QString &location)
{
    if (m_location != location) {
        m_values.remove("printer-location");
    } else {
        m_values["printer-location"] = location;
    }
}

void Printer::setConnection(const QString &connection)
{
    if (m_connection != connection) {
        m_values.remove("device-uri");
    } else {
        m_values["device-uri"] = connection;
    }
}

void Printer::setMakeAndModel(const QString &makeAndModel)
{
    if (m_makeAndModel != makeAndModel) {
        m_values.remove("ppd-name");
    } else {
        m_values["ppd-name"] = makeAndModel;
    }
}

void Printer::setShared(bool shared)
{
    if (m_shared != shared) {
        m_values.remove("printer-is-shared");
    } else {
        m_values["printer-is-shared"] = shared;
    }
}

QString Printer::description() const
{
    return m_description;
}

QString Printer::location() const
{
    return m_location;
}

QString Printer::connection() const
{
    return m_connection;
}

QString Printer::makeAndModel() const
{
    return m_makeAndModel;
}

bool Printer::shared() const
{
    return m_shared;
}

bool Printer::save()
{
    if (m_values.size() > 0) {
        return false;
    }

    RUN_ACTION(cupsAddModifyPrinter(m_destName.toLocal8Bit(), m_values))
}

bool Printer::setShared(const QString &destName, bool shared)
{
    QHash<QString, QVariant> values;
    values["printer-is-shared"] = shared;
    RUN_ACTION(cupsAddModifyPrinter(destName.toLocal8Bit(), values))
}

#include "QCups.moc"
