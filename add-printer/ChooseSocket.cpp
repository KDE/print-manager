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

#include "ChooseSocket.h"
#include "ui_ChooseSocket.h"

#include <QPainter>
#include <KDebug>

ChooseSocket::ChooseSocket(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseSocket),
    m_isValid(false)
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
    // loads the standard key icon
//    QPixmap pixmap;
//    pixmap = KIconLoader::global()->loadIcon("printer",
//                                             KIconLoader::NoGroup,
//                                             KIconLoader::SizeEnormous, // a not so huge icon
//                                             KIconLoader::DefaultState);
//    QPixmap icon(pixmap);
//    QPainter painter(&icon);

//    pixmap = KIconLoader::global()->loadIcon("network-wired",
//                                             KIconLoader::NoGroup,
//                                             KIconLoader::SizeLarge, // a not so huge icon
//                                             KIconLoader::DefaultState);
//    // the the emblem icon to size 32
//    int overlaySize = KIconLoader::SizeLarge;
//    QPoint startPoint;
//    // bottom right corner
//    startPoint = QPoint(KIconLoader::SizeEnormous - overlaySize - 2,
//                        KIconLoader::SizeEnormous - overlaySize - 2);
//    painter.drawPixmap(startPoint, pixmap);
//    ui->printerL->setPixmap(icon);
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
    QString deviceUri = args[DEVICE_URI].toString();
    KUrl url = deviceUri;
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
    KUrl url = KUrl(QLatin1String("socket://") + ui->addressLE->text());
    url.setPort(ui->portISB->value());
    ret[DEVICE_URI] = url.prettyUrl();
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
