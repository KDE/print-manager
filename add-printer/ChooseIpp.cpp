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

#include "ChooseIpp.h"
#include "ui_ChooseIpp.h"

#include <QPainter>
#include <KDebug>

ChooseIpp::ChooseIpp(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseIpp),
    m_isValid(false)
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
    // loads the standard key icon
    QPixmap pixmap;
    pixmap = KIconLoader::global()->loadIcon("printer",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeEnormous, // a not so huge icon
                                             KIconLoader::DefaultState);
    QPixmap icon(pixmap);
    QPainter painter(&icon);

    pixmap = KIconLoader::global()->loadIcon("network-wired",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeLarge, // a not so huge icon
                                             KIconLoader::DefaultState);
    // the the emblem icon to size 32
    int overlaySize = KIconLoader::SizeLarge;
    QPoint startPoint;
    // bottom right corner
    startPoint = QPoint(KIconLoader::SizeEnormous - overlaySize - 2,
                        KIconLoader::SizeEnormous - overlaySize - 2);
    painter.drawPixmap(startPoint, pixmap);
    ui->printerL->setPixmap(icon);

    connect(ui->addressLE, SIGNAL(textChanged(QString)), this, SLOT(verifyURL()));
    connect(ui->queueLE, SIGNAL(textChanged(QString)), this, SLOT(verifyURL()));
}

ChooseIpp::~ChooseIpp()
{
    delete ui;
}

void ChooseIpp::setValues(const QVariantHash &args)
{
    if (m_args == args) {
        return;
    }

    m_args = args;
    QString deviceUri = args["device-uri"].toString();
    if (deviceUri.contains('/')) {
        kDebug() << deviceUri;
        KUrl url = deviceUri;
        ui->addressLE->setText(url.authority());
        ui->queueLE->setText(url.path());
    } else {
        ui->addressLE->clear();
        ui->queueLE->setText("/printers/");
    }

    m_isValid = true;
}

QVariantHash ChooseIpp::values() const
{
    QVariantHash ret = m_args;
    // Ipp can be ipp, http and https
    ret["device-uri"] = uri();
    return ret;
}

bool ChooseIpp::isValid() const
{
    return m_isValid;
}

QString ChooseIpp::uri() const
{
    QString ret;
    if (!ui->addressLE->text().isEmpty() && ui->queueLE->text() != "/printers/") {
        QString queue = ui->queueLE->text();
        if (!queue.startsWith('/')) {
            queue.prepend('/');
        }
        ret = QString("%1://%2%3")
              .arg(m_args["device-uri"].toString())
              .arg(ui->addressLE->text())
              .arg(queue);
    }
    return ret;
}

bool ChooseIpp::canProceed() const
{
    return !uri().isEmpty();
}

void ChooseIpp::verifyURL()
{
    bool allow = canProceed();
    ui->uriL->setText(uri());
    ui->uriL->setVisible(allow);
    emit allowProceed(allow);
}
