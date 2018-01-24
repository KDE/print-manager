/***************************************************************************
 *   Copyright (C) 2010-2018 by Daniel Nicoletti                           *
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

#include "ChooseUri.h"
#include "ui_ChooseUri.h"

#include <KCupsRequest.h>

#include <QStringBuilder>
#include <QDebug>

ChooseUri::ChooseUri(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseUri)
{
    ui->setupUi(this);

    ui->searchTB->setIcon(QIcon::fromTheme("edit-find"));

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    connect(ui->addressLE, &QLineEdit::textChanged, this, &ChooseUri::checkSelected);
    connect(ui->addressLE, &QLineEdit::returnPressed, this, &ChooseUri::findPrinters);
    connect(ui->searchTB, &QToolButton::clicked, this, &ChooseUri::findPrinters);
}

ChooseUri::~ChooseUri()
{
    delete ui;
}

void ChooseUri::setValues(const QVariantHash &args)
{
    m_args = args;
    bool visible = false;
    QUrl url(args[KCUPS_DEVICE_URI].toString());
    if (url.url() == QLatin1String("other")) {
        ui->addressLE->clear();
        visible = true;
    } else if (url.scheme().isEmpty() && url.authority().isEmpty()) {
        ui->addressLE->setText(url.url() % QLatin1String("://"));
    } else {
        ui->addressLE->setText(url.url());
    }
    ui->searchTB->setVisible(visible);
    ui->addressLE->setFocus();
}

QVariantHash ChooseUri::values() const
{
    QVariantHash ret = m_args;

    ret[KCUPS_DEVICE_URI] = parsedURL(ui->addressLE->text()).url();

    return ret;
}

bool ChooseUri::isValid() const
{
    QVariantHash args = values();
    QUrl url(args[KCUPS_DEVICE_URI].toString());
    //qDebug() << url << url.isValid() << url.isEmpty() << url.scheme().isEmpty() << url.host();
    return url.isValid() && !url.isEmpty() && !url.scheme().isEmpty() && !url.host().isEmpty();
}

bool ChooseUri::canProceed() const
{
    return isValid();
}

void ChooseUri::load()
{
}

void ChooseUri::checkSelected()
{
    emit allowProceed(isValid());
}

void ChooseUri::on_addressLE_textChanged(const QString &text)
{
    QUrl url = parsedURL(text);

    if (url.isValid() &&
            (url.scheme().isEmpty() ||
             url.scheme() == QStringLiteral("http") ||
             url.scheme() == QStringLiteral("https") ||
             url.scheme() == QStringLiteral("ipp"))) {
        // TODO maybe cups library can connect to more protocols
        ui->searchTB->setEnabled(true);
    } else {
        ui->searchTB->setEnabled(false);
    }
}

void ChooseUri::findPrinters()
{
    QUrl url = parsedURL(ui->addressLE->text());

    auto conn = new KCupsConnection(url, this);
    auto request = new KCupsRequest(conn);
    connect(request, &KCupsRequest::finished, this, &ChooseUri::getPrintersFinished);

    QStringList attr;
    attr << KCUPS_PRINTER_NAME;
    attr << KCUPS_PRINTER_STATE;
    attr << KCUPS_PRINTER_IS_SHARED;
    attr << KCUPS_PRINTER_IS_ACCEPTING_JOBS;
    attr << KCUPS_PRINTER_TYPE;
    attr << KCUPS_PRINTER_LOCATION;
    attr << KCUPS_PRINTER_INFO;
    attr << KCUPS_PRINTER_MAKE_AND_MODEL;
    request->setProperty("URI", url);

    emit startWorking();
    request->getPrinters(attr);
}

void ChooseUri::getPrintersFinished(KCupsRequest *request)
{
    QUrl uri = request->property("URI").value<QUrl>();
    QUrl url;
    url.setScheme(QStringLiteral("ipp"));
    url.setAuthority(uri.authority());

    KCupsPrinters printers = request->printers();
    if (request->hasError()) {
        emit errorMessage(request->errorMsg());
    } else {
        emit insertDevice("network",
                          url.authority(),
                          url.authority(),
                          QString(),
                          url.url(),
                          QString(),
                          printers);
    }

    request->deleteLater();
    request->connection()->deleteLater();
    emit stopWorking();
}

QUrl ChooseUri::parsedURL(const QString &text) const
{
    QUrl url(QUrl::fromUserInput(text));
    if (url.host().isEmpty() && !text.contains(QLatin1String("://"))) {
        url = QUrl();
        // URI might be scsi, network on anything that didn't match before
        if (m_args[KCUPS_DEVICE_URI].toString() != QLatin1String("other")) {
            url.setScheme(m_args[KCUPS_DEVICE_URI].toString());
        }
        url.setAuthority(text);
    }
    return url;
}
