/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "ChooseSamba.h"
#include "ui_ChooseSamba.h"

#include <KCupsRequest.h>

#include <QPainter>
#include <QStringBuilder>

#include <KUrl>

#include <KDebug>

ChooseSamba::ChooseSamba(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseSamba)
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    connect(ui->addressLE, SIGNAL(textChanged(QString)), this, SLOT(checkSelected()));
    connect(ui->usernameLE, SIGNAL(textChanged(QString)), this, SLOT(checkSelected()));
    connect(ui->passwordLE, SIGNAL(textChanged(QString)), this, SLOT(checkSelected()));
}

ChooseSamba::~ChooseSamba()
{
    delete ui;
}

void ChooseSamba::setValues(const QVariantHash &args)
{
    m_args = args;
    ui->addressLE->setFocus();
}

QVariantHash ChooseSamba::values() const
{
    QVariantHash ret = m_args;

    QString address = ui->addressLE->text().trimmed();
    KUrl url;
    if (address.startsWith(QLatin1String("//"))) {
        url = QLatin1String("smb:") % address;
    } else if (address.startsWith(QLatin1String("/"))) {
        url = QLatin1String("smb:/") % address;
    } else if (address.startsWith(QLatin1String("://"))) {
        url = QLatin1String("smb") % address;
    } else if (address.startsWith(QLatin1String("smb://"))) {
        url = address;
    } else if (!KUrl(address).protocol().isEmpty() &&
               KUrl(address).protocol() != QLatin1String("smb")) {
        url = address;
        url.setProtocol(QLatin1String("smb"));
    } else {
        url = QLatin1String("smb://") % address;
    }

    kDebug() << 1 << url;
    if (!ui->usernameLE->text().isEmpty()) {
        url.setUser(ui->usernameLE->text());
    }

    if (!ui->passwordLE->text().isEmpty()) {
        url.setPass(ui->passwordLE->text());
    }

    kDebug() << 2 << url;
    kDebug() << 3 << url.url() << url.path().section(QLatin1Char('/'), -1, -1);// same as url.fileName()
    kDebug() << 4 << url.fileName();
    kDebug() << 5 << url.host() << url.url().section(QLatin1Char('/'), 3, 3).toLower();

    ret[KCUPS_DEVICE_URI] = url.url();
    ret[KCUPS_DEVICE_INFO] = url.fileName();

    // if there is 4 '/' means the url is like
    // smb://group/host/printer, so the location is at a different place
    if (url.url().count(QLatin1Char('/') == 4)) {
        ret[KCUPS_DEVICE_LOCATION] = url.url().section(QLatin1Char('/'), 3, 3).toLower();
    } else {
        ret[KCUPS_DEVICE_LOCATION] = url.host();
    }

    return ret;
}

bool ChooseSamba::isValid() const
{
    QVariantHash args = values();
    KUrl url(args[KCUPS_DEVICE_URI].toString());

    return url.isValid() &&
            !url.isEmpty() &&
            !url.protocol().isEmpty() &&
            url.hasHost() &&
            url.hasPath() &&
            !url.fileName().isEmpty() &&
            url.url().count(QLatin1Char('/')) <= 4;
}

bool ChooseSamba::canProceed() const
{
    return isValid();
}

void ChooseSamba::load()
{
}

void ChooseSamba::checkSelected()
{
    emit allowProceed(isValid());
}
