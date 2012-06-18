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

#include "ChooseLpd.h"
#include "ui_ChooseLpd.h"

#include <QPainter>
#include <QStringBuilder>

#include <KDebug>

ChooseLpd::ChooseLpd(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseLpd),
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

ChooseLpd::~ChooseLpd()
{
    delete ui;
}

void ChooseLpd::on_addressLE_textChanged(const QString &text)
{
    kDebug() << text;
}

void ChooseLpd::setValues(const QVariantHash &args)
{
    m_args = args;
    QString deviceUri = args[DEVICE_URI].toString();
    kDebug() << deviceUri;
    if (deviceUri.contains(QLatin1Char('/'))) {
        m_isValid = false;
        return;
    }
    m_isValid = true;

    ui->addressLE->setText(deviceUri);
    ui->addressLE->setFocus();
}

QVariantHash ChooseLpd::values() const
{
    QVariantHash ret = m_args;
    ret[DEVICE_URI] = static_cast<QString>(QLatin1String("lpd://") % ui->addressLE->text());
    return ret;
}

bool ChooseLpd::canProceed() const
{
    bool allow = false;
    if (!ui->addressLE->text().isEmpty()) {
        KUrl url = KUrl(QLatin1String("lpd://") % ui->addressLE->text());
        allow = url.isValid();
    }
    return allow;
}

bool ChooseLpd::isValid() const
{
    return m_isValid;
}

void ChooseLpd::checkSelected()
{
//     emit allowProceed(!devicesLV->selectionModel()->selection().isEmpty());
}
