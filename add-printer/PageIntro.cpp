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

#include "PageIntro.h"

#include <QPainter>

#include <KDebug>

PageIntro::PageIntro(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageIntro),
    m_addPrinter(true)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Welcome to the add printer wizard"));
    // loads the standard key icon
    QPixmap pixmap;
    pixmap = KIconLoader::global()->loadIcon("computer",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeEnormous, // a not so huge icon
                                             KIconLoader::DefaultState);
    QPixmap icon(pixmap);
    QPainter painter(&icon);

    pixmap = KIconLoader::global()->loadIcon("applications-other.png",
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
    ui->computerL->setPixmap(icon);
//     softwareL->setPixmap(pixmap);

    pixmap = KIconLoader::global()->loadIcon("printer",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeEnormous, // a not so huge icon
                                             KIconLoader::DefaultState);

    QPixmap icon2(pixmap);
    pixmap = KIconLoader::global()->loadIcon("tools-wizard",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeLarge, // a not so huge icon
                                             KIconLoader::DefaultState);
    QPainter painter2(&icon2);
    // the the emblem icon to size 32
//      overlaySize = KIconLoader::SizeLarge;
//     QPoint startPoint;
    // bottom right corner
    startPoint = QPoint(KIconLoader::SizeEnormous - overlaySize - 2,
                        KIconLoader::SizeEnormous - overlaySize - 2);
    painter2.drawPixmap(startPoint, pixmap);
    ui->printerL->setPixmap(icon2);

    connect(ui->addNewPrinterCB, SIGNAL(clicked()),
            this, SIGNAL(next()));
    connect(ui->addNewClassCB, SIGNAL(clicked()),
            this, SIGNAL(next()));
}

PageIntro::~PageIntro()
{
    delete ui;
}

bool PageIntro::canProceed() const
{
    // Return false as we want the user
    // to use the command buttons
    return false;
}

bool PageIntro::hasChanges() const
{
    return m_args["add-new-printer"].toBool() != m_addPrinter;
}

QVariantHash PageIntro::values() const
{
    QVariantHash ret = m_args;
    ret["add-new-printer"] = m_addPrinter;
    kDebug() << ret;
    return ret;
}

void PageIntro::on_addNewPrinterCB_clicked()
{
    m_addPrinter = true;
}

void PageIntro::on_addNewClassCB_clicked()
{
    m_addPrinter = false;
}
