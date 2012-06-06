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

#include "ChooseSamba.h"
#include "ui_ChooseSamba.h"

#include <QPainter>
#include <QStringBuilder>

#include <KDebug>

ChooseSamba::ChooseSamba(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseSamba),
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

    pixmap = KIconLoader::global()->loadIcon("kde-windows",
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
}

ChooseSamba::~ChooseSamba()
{
    delete ui;
}

void ChooseSamba::setValues(const QVariantHash &args)
{
    m_args = args;
    QString deviceUri = args[DEVICE_URI].toString();
    if (deviceUri.contains(QLatin1Char('/'))) {
        m_isValid = false;
        return;
    }
    m_isValid = true;

    ui->addressLE->setText(deviceUri);
}

QVariantHash ChooseSamba::values() const
{
    QVariantHash ret = m_args;
    ret[DEVICE_URI] = static_cast<QString>(QLatin1String("smb://") % ui->addressLE->text());
    return ret;
}

bool ChooseSamba::isValid() const
{
    return m_isValid;
}

bool ChooseSamba::canProceed() const
{
    bool allow = false;
    if (!ui->addressLE->text().isEmpty()) {
        KUrl url = KUrl(QLatin1String("smb://") % ui->addressLE->text());
        allow = url.isValid();
    }
    return allow;
}

void ChooseSamba::load()
{
}

void ChooseSamba::on_detectPB_clicked()
{
}

void ChooseSamba::checkSelected()
{
//     emit allowProceed(!devicesLV->selectionModel()->selection().isEmpty());
}
