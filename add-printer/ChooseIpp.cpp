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

#include <QPainter>
#include <KDebug>

ChooseIpp::ChooseIpp(QWidget *parent)
 : GenericPage(parent), m_isValid(false)
{
    setupUi(this);

    // setup default options
    setWindowTitle(i18n("Select a Printer to Add"));
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
    printerL->setPixmap(icon);
}

ChooseIpp::~ChooseIpp()
{
}

void ChooseIpp::setValues(const QHash<QString, QVariant> &args)
{
    if (m_args == args) {
        return;
    }

    m_args = args;
    QString deviceUri = args["device-uri"].toString();
    if (deviceUri.contains('/')) {
        kDebug() << deviceUri;
        QUrl url = deviceUri;
        addressLE->setText(url.authority());
        queueLE->setText(url.path());
    } else {
        addressLE->clear();
        queueLE->setText("/printers/");
    }
//     if (deviceUri.contains('/')) {
//         m_isValid = false;
//         return;
//     }
    m_isValid = true;
}

bool ChooseIpp::isValid() const
{
    return m_isValid;
}

void ChooseIpp::load()
{
}

void ChooseIpp::on_detectPB_clicked()
{
}

void ChooseIpp::checkSelected()
{
//     emit allowProceed(!devicesLV->selectionModel()->selection().isEmpty());
}

#include "ChooseIpp.moc"
