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

#include "PageChoosePPD.h"

#include "DevicesModel.h"
#include <SelectMakeModel.h>

#include <QPainter>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KDebug>
#include <KPixmapSequence>

PageChoosePPD::PageChoosePPD(QWidget *parent)
 : GenericPage(parent),
   m_isValid(false)
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

    pixmap = KIconLoader::global()->loadIcon("page-zoom",
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


    m_layout = new QStackedLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addLayout(m_layout, 1, 3);
    SelectMakeModel *widget = new SelectMakeModel(this);
//     widget->setMakeModel(QString(), QString());
    m_layout->addWidget(widget);

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//     m_busySeq->setWidget(devicesLV->viewport());
}

PageChoosePPD::~PageChoosePPD()
{
}

void PageChoosePPD::setValues(const QHash<QString, QVariant> &args)
{
    m_args = args;
    if (args["add-new-printer"].toBool()) {
        m_isValid = true;
//         m_busySeq->start();
    } else {
        m_isValid = false;
    }
}

bool PageChoosePPD::isValid() const
{
    return m_isValid;
}

bool PageChoosePPD::hasChanges() const
{
    if (!isValid()) {
        return false;
    }

    QString deviceURI;
    if (canProceed()) {
//         deviceURI = devicesLV->selectionModel()->selectedIndexes().first().data(DevicesModel::DeviceURI).toString();
    }
    return deviceURI != m_args["device-uri"];
}

QHash<QString, QVariant> PageChoosePPD::values() const
{
    if (!isValid()) {
        return m_args;
    }

    QHash<QString, QVariant> ret = m_args;
    if (canProceed()) {
//         QModelIndex index = devicesLV->selectionModel()->selectedIndexes().first();
//         kDebug() << index.data(DevicesModel::DeviceURI).toString();
//         ret["device-uri"] = index.data(DevicesModel::DeviceURI).toString();
//         ret["device-make-and-model"] = index.data(DevicesModel::DeviceMakeAndModel).toString();
//         ret["device-info"] = index.data(DevicesModel::DeviceInfo).toString();
    }
    return ret;
}

bool PageChoosePPD::canProceed() const
{
    // It can proceed if one and JUST one item is selected
    // (if the user clicks on the category all items in it get selected)
//     return (!devicesLV->selectionModel()->selectedIndexes().isEmpty() &&
//              devicesLV->selectionModel()->selectedIndexes().size() == 1);
return false;
}

void PageChoosePPD::checkSelected()
{
    emit allowProceed(canProceed());
}

#include "PageChoosePPD.moc"
