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

#include "PageDestinations.h"
#include "ui_PageDestinations.h"

#include "DevicesModel.h"

#include <QPainter>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KDebug>
#include <KPixmapSequence>

// system-config-printer --setup-printer='file:/tmp/printout' --devid='MFG:Ricoh;MDL:Aficio SP C820DN'
PageDestinations::PageDestinations(const QVariantHash &args, QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageDestinations),
    m_isValid(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

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
    ui->printerL->setPixmap(icon);

    m_model = new DevicesModel(this);
    KCategorizedSortFilterProxyModel *proxy = new KCategorizedSortFilterProxyModel(m_model);
    proxy->setSourceModel(m_model);
    proxy->setCategorizedModel(true);
    proxy->setDynamicSortFilter(true);
    proxy->sort(0);
    KCategoryDrawerV3 *drawer = new KCategoryDrawerV3(ui->devicesLV);
    ui->devicesLV->setModel(proxy);
    ui->devicesLV->setCategoryDrawer(drawer);
    connect(ui->devicesLV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(checkSelected()));
    connect(ui->devicesLV, SIGNAL(activated(QModelIndex)),
            this, SIGNAL(proceed()));

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    m_busySeq->setWidget(ui->printerL);
    connect(m_model, SIGNAL(loaded()), m_busySeq, SLOT(stop()));

    // set our args
    setValues(args);
}

PageDestinations::~PageDestinations()
{
    delete ui;
}

void PageDestinations::setValues(const QVariantHash &args)
{
    m_args = args;
    if (args[ADDING_PRINTER].toBool()) {
        m_isValid = true;
        m_model->update();
        m_busySeq->start();
    } else {
        m_isValid = false;
    }
}

bool PageDestinations::isValid() const
{
    return m_isValid;
}

bool PageDestinations::hasChanges() const
{
    if (!isValid()) {
        return false;
    }

    QString deviceURI;
    if (canProceed()) {
        deviceURI = ui->devicesLV->selectionModel()->selectedIndexes().first().data(DevicesModel::DeviceUri).toString();
    }
    return deviceURI != m_args[DEVICE_URI];
}

QVariantHash PageDestinations::values() const
{
    if (!isValid()) {
        return m_args;
    }

    QVariantHash ret = m_args;
    if (canProceed()) {
        QModelIndex index = ui->devicesLV->selectionModel()->selectedIndexes().first();
        ret[DEVICE_URI] = index.data(DevicesModel::DeviceUri).toString();
        ret[DEVICE_ID] = index.data(DevicesModel::DeviceId).toString();
        ret[DEVICE_MAKE_MODEL] = index.data(DevicesModel::DeviceMakeAndModel).toString();
        ret[DEVICE_INFO] = index.data(DevicesModel::DeviceInfo).toString();
        ret[DEVICE_LOCATION] = index.data(DevicesModel::DeviceLocation).toString();
        kDebug() << ret;
    }
    return ret;
}

bool PageDestinations::canProceed() const
{
    // It can proceed if one and JUST one item is selected
    // (if the user clicks on the category all items in it get selected)
    return (!ui->devicesLV->selectionModel()->selectedIndexes().isEmpty() &&
             ui->devicesLV->selectionModel()->selectedIndexes().size() == 1);
}

void PageDestinations::checkSelected()
{
    emit allowProceed(canProceed());
}
