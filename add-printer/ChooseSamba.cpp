/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ChooseSamba.h"
#include "ui_ChooseSamba.h"

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QPainter>
#include <QDebug>
#include <QUrl>

ChooseSamba::ChooseSamba(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseSamba)
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    connect(ui->addressLE, &QLineEdit::textChanged, this, &ChooseSamba::checkSelected);
    connect(ui->usernameLE, &QLineEdit::textChanged, this, &ChooseSamba::checkSelected);
    connect(ui->passwordLE, &QLineEdit::textChanged, this, &ChooseSamba::checkSelected);
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

    const QString address = ui->addressLE->text().trimmed();
    QUrl url;
    if (address.startsWith(QLatin1String("//"))) {
        url = QUrl(QLatin1String("smb:") + address);
    } else if (address.startsWith(QLatin1String("/"))) {
        url = QUrl(QLatin1String("smb:/") + address);
    } else if (address.startsWith(QLatin1String("://"))) {
        url = QUrl(QLatin1String("smb") + address);
    } else if (address.startsWith(QLatin1String("smb://"))) {
        url = QUrl(address);
    } else if (!QUrl::fromUserInput(address).scheme().isEmpty() &&
               QUrl::fromUserInput(address).scheme() != QStringLiteral("smb")) {
        url = QUrl::fromUserInput(address);
        url.setScheme(QStringLiteral("smb"));
    } else {
        url = QUrl(QStringLiteral("smb://") + address);
    }

    qDebug() << 1 << url;
    if (!ui->usernameLE->text().isEmpty()) {
        url.setUserName(ui->usernameLE->text());
    }

    if (!ui->passwordLE->text().isEmpty()) {
        url.setPassword(ui->passwordLE->text());
    }

    qDebug() << 2 << url;
    qDebug() << 3 << url.url() << url.path().section(QLatin1Char('/'), -1, -1);// same as url.fileName()
    qDebug() << 4 << url.fileName();
    qDebug() << 5 << url.host() << url.url().section(QLatin1Char('/'), 3, 3).toLower();

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
    const QVariantHash args = values();
    const QUrl url(args[KCUPS_DEVICE_URI].toString());

    return url.isValid() &&
            !url.isEmpty() &&
            !url.scheme().isEmpty() &&
            !url.host().isEmpty() &&
            !url.path().isEmpty() &&
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

#include "moc_ChooseSamba.cpp"
