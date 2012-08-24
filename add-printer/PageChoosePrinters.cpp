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

#include "PageChoosePrinters.h"
#include "ui_PageChoosePrinters.h"

#include <ClassListWidget.h>

#include <KCupsRequest.h>

#include <QPainter>
#include <KDebug>

PageChoosePrinters::PageChoosePrinters(const QVariantHash &args, QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageChoosePrinters)
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

    pixmap = KIconLoader::global()->loadIcon("preferences-other",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeLarge, // a not so huge icon
                                             KIconLoader::DefaultState);
    // the emblem icon to size 32
    int overlaySize = KIconLoader::SizeLarge;
    QPoint startPoint;
    // bottom right corner
    startPoint = QPoint(KIconLoader::SizeEnormous - overlaySize - 2,
                        KIconLoader::SizeEnormous - overlaySize - 2);
    painter.drawPixmap(startPoint, pixmap);
    ui->printerL->setPixmap(icon);

    connect(ui->membersLV, SIGNAL(changed(bool)),
            this, SIGNAL(allowProceed(bool)));

    if (!args.isEmpty()) {
        setValues(args);
    }
}

PageChoosePrinters::~PageChoosePrinters()
{
    delete ui;
}

void PageChoosePrinters::setValues(const QVariantHash &args)
{
    if (m_args != args) {
        ui->membersLV->reload(QString());
        m_args = args;
    }
}

QVariantHash PageChoosePrinters::values() const
{
    QVariantHash ret = m_args;
    ret[KCUPS_MEMBER_URIS] = ui->membersLV->selectedDests();
    return ret;
}

bool PageChoosePrinters::canProceed() const
{
    return ui->membersLV->selectedDests().count() > 0;
}
