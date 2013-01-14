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

#include "ChooseUri.h"
#include "ui_ChooseUri.h"

#include <KCupsRequest.h>

#include <KUrl>
#include <QStringBuilder>

#include <KDebug>

ChooseUri::ChooseUri(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseUri)
{
    ui->setupUi(this);

    ui->searchTB->setIcon(KIcon("edit-find"));

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    connect(ui->addressLE, SIGNAL(textChanged(QString)), this, SLOT(checkSelected()));
    connect(ui->addressLE, SIGNAL(returnPressed()), this, SLOT(findPrinters()));
    connect(ui->searchTB, SIGNAL(clicked()), this, SLOT(findPrinters()));
}

ChooseUri::~ChooseUri()
{
    delete ui;
}

void ChooseUri::setValues(const QVariantHash &args)
{
    m_args = args;
    bool visible = false;
    KUrl url = args[KCUPS_DEVICE_URI].toString();
    if (url.url() == QLatin1String("other")) {
        ui->addressLE->clear();
        visible = true;
    } else if (url.protocol().isEmpty() && url.authority().isEmpty()) {
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
    KUrl url(args[KCUPS_DEVICE_URI].toString());
//kDebug() << url << url.isValid() << url.isEmpty() << url.protocol().isEmpty() << url.hasHost();
    return url.isValid() && !url.isEmpty() && !url.protocol().isEmpty() && url.hasHost();
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
    KUrl url = parsedURL(text);

    if (url.isValid() &&
            (url.protocol().isEmpty() ||
             url.protocol() == QLatin1String("http") ||
             url.protocol() == QLatin1String("https") ||
             url.protocol() == QLatin1String("ipp"))) {
        // TODO maybe cups library can connect to more protocols
        ui->searchTB->setEnabled(true);
    } else {
        ui->searchTB->setEnabled(false);
    }
}

void ChooseUri::findPrinters()
{
    KUrl url = parsedURL(ui->addressLE->text());

    KCupsConnection *conn = new KCupsConnection(url, this);
    KCupsRequest *request = new KCupsRequest(conn);
    connect(request, SIGNAL(device(QString,QString,QString,QString,QString,QString)),
            this, SLOT(gotDevice(QString,QString,QString,QString,QString,QString)));
    connect(request, SIGNAL(finished()), this, SLOT(getPrintersFinished()));

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
    request->getPrinters(attr);
}

void ChooseUri::gotDevice(const QString &device_class, const QString &device_id, const QString &device_info, const QString &device_make_and_model, const QString &device_uri, const QString &device_location)
{
    kDebug() << device_class;
    // "MFG:Samsung;CMD:GDI;MDL:SCX-4200 Series;CLS:PRINTER;MODE:PCL;STATUS:IDLE;"
    kDebug() << device_id;
    // "Samsung SCX-4200 Series"
    kDebug() << device_info;
    // "Samsung SCX-4200 Series"
    kDebug() << device_make_and_model;
    // "usb://Samsung/SCX-4200%20Series"
    kDebug() << device_uri;
    // ""
    kDebug() << device_location;

}

void ChooseUri::getPrintersFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    KUrl url = request->property("URI").value<KUrl>();

    QStringList uris;
    foreach (const KCupsPrinter &printer, request->printers()) {
        KUrl printerURI;
        printerURI.setProtocol(QLatin1String("ipp"));
        printerURI.setAuthority(url.authority());
        printerURI.addPath(QLatin1String("printers/") % printer.name());
        uris << printerURI.url();
    }

    emit insertDevice("network",
                      url.authority(),
                      url.authority(),
                      QString(),
                      url.url(),
                      QString(),
                      uris);

    request->deleteLater();
    request->connection()->deleteLater();
}

void ChooseUri::gotPPDFinished()
{
    kDebug();
    KCupsRequest *request = qobject_cast<KCupsRequest*>(sender());
    kDebug() << request->printerPPD();
}

KUrl ChooseUri::parsedURL(const QString &text) const
{
    KUrl url(text);
    if (url.host().isEmpty() && !text.contains(QLatin1String("://"))) {
        url = KUrl();
        // URI might be scsi, network on anything that didn't match before
        if (m_args[KCUPS_DEVICE_URI].toString() != QLatin1String("other")) {
            url.setProtocol(m_args[KCUPS_DEVICE_URI].toString());
        }
        url.setAuthority(text);
    }
    return url;
}
