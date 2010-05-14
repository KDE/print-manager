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

#include "PageAddPrinter.h"

#include <QPainter>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KDebug>

PageAddPrinter::PageAddPrinter(QWidget *parent)
 : GenericPage(parent)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

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

    pixmap = KIconLoader::global()->loadIcon("dialog-information",
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

PageAddPrinter::~PageAddPrinter()
{
}

void PageAddPrinter::setValues(const QHash<QString, QString> &args)
{
    m_args = args;
    QString name = args["device-info"];
    name.replace(' ', '_');
    nameLE->setText(name);
    descriptionLE->setText(args["device-info"]);
    locationLE->clear();
    shareCB->setChecked(true);
}

void PageAddPrinter::load()
{
}

void PageAddPrinter::checkSelected()
{
//     emit allowProceed(!devicesLV->selectionModel()->selection().isEmpty());
}

#include "PageAddPrinter.moc"
