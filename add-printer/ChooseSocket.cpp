/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ChooseSocket.h"
#include "ui_ChooseSocket.h"

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QPainter>
#include <QUrl>

ChooseSocket::ChooseSocket(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseSocket)
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
}

ChooseSocket::~ChooseSocket()
{
    delete ui;
}

void ChooseSocket::setValues(const QVariantHash &args)
{
    if (m_args == args) {
        return;
    }

    m_args = args;
    ui->addressLE->clear();
    ui->portISB->setValue(9100);
    const QString deviceUri = args[KCUPS_DEVICE_URI].toString();
    QUrl url(deviceUri);
    if (url.scheme() == QLatin1String("socket")) {
        ui->addressLE->setText(url.host());
        ui->portISB->setValue(url.port(9100));
    }
    ui->addressLE->setFocus();

    m_isValid = true;
}

QVariantHash ChooseSocket::values() const
{
    QVariantHash ret = m_args;
    QUrl url = QUrl(QLatin1String("socket://") + ui->addressLE->text());
    url.setPort(ui->portISB->value());
    ret[KCUPS_DEVICE_URI] = url.toDisplayString();
    return ret;
}

bool ChooseSocket::isValid() const
{
    return m_isValid;
}

bool ChooseSocket::canProceed() const
{
    return !ui->addressLE->text().isEmpty();
}

void ChooseSocket::on_addressLE_textChanged(const QString &text)
{
    Q_UNUSED(text)
    emit allowProceed(canProceed());
}

#include "moc_ChooseSocket.cpp"
