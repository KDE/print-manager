/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KCupsPasswordDialog.h"

#include "Debug.h"

#include <QPointer>

#include <KPasswordDialog>
#include <KWindowSystem>
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
#include <KX11Extras>
#endif
#include <KLocalizedString>

KCupsPasswordDialog::KCupsPasswordDialog(QObject *parent) :
    QObject(parent),
    m_accepted(false),
    m_mainwindow(0),
    // default text, can be overridden using setPromptText()
    m_promptText(i18n("Enter an username and a password to complete the task"))
{
}

void KCupsPasswordDialog::setMainWindow(WId mainwindow)
{
    m_mainwindow = mainwindow;
}

void KCupsPasswordDialog::setPromptText(const QString &text)
{
    m_promptText = text;
}

void KCupsPasswordDialog::exec(const QString &username, bool wrongPassword)
{
    QPointer<KPasswordDialog> dialog = new KPasswordDialog(nullptr, KPasswordDialog::ShowUsernameLine);
    dialog->setPrompt(m_promptText);
    dialog->setModal(false);
    dialog->setUsername(username);
    if (wrongPassword) {
        dialog->showErrorMessage(QString(), KPasswordDialog::UsernameError);
        dialog->showErrorMessage(i18n("Wrong username or password"), KPasswordDialog::PasswordError);
    }

    dialog->show();
    if (m_mainwindow) {
        dialog->setAttribute(Qt::WA_NativeWindow, true);
        KWindowSystem::setMainWindow(dialog->windowHandle(), m_mainwindow);
    }
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    KX11Extras::forceActiveWindow(dialog->winId());
#else
    KWindowSystem::forceActiveWindow(dialog->winId());
#endif

    // Do not return from this method now
    dialog->exec();

    if (dialog) {
        m_accepted = dialog->result() == QDialog::Accepted;
        m_username = dialog->username();
        m_password = dialog->password();
        dialog->deleteLater();
    }
}

bool KCupsPasswordDialog::accepted() const
{
    return m_accepted;
}

QString KCupsPasswordDialog::username() const
{
    return m_username;
}

QString KCupsPasswordDialog::password() const
{
    return m_password;
}

#include "moc_KCupsPasswordDialog.cpp"
