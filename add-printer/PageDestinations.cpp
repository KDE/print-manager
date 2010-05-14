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

#include "DevicesModel.h"

#include <QPainter>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KDebug>


// TODO KPixmapSequence KPixmapSequenceWidget
// gwenview floater, jockey proprietary dirvers
// system-config-printer --setup-printer='file:/tmp/printout' --devid='MFG:Ricoh;MDL:Aficio SP C820DN'
PageDestinations::PageDestinations(QWidget *parent)
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

    m_model = new DevicesModel(this);
    KCategorizedSortFilterProxyModel *proxy = new KCategorizedSortFilterProxyModel(m_model);
    proxy->setSourceModel(m_model);
    proxy->setCategorizedModel(true);
    proxy->setDynamicSortFilter(true);
    proxy->sort(0);
    KCategoryDrawerV2 *drawer = new KCategoryDrawerV2(devicesLV);
    devicesLV->setModel(proxy);
    devicesLV->setCategoryDrawer(drawer);
    connect(devicesLV->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(checkSelected()));
}

PageDestinations::~PageDestinations()
{
}

void PageDestinations::setValues(const QHash<QString, QString> &args)
{
    m_args = args;
    m_model->update();
}

bool PageDestinations::hasChanges()
{
    QString deviceURI;
    if (canProceed()) {
        deviceURI = devicesLV->selectionModel()->selectedIndexes().first().data(DevicesModel::DeviceURI).toString();
    }
    return deviceURI != m_args["device-uri"];
}

QHash<QString, QString> PageDestinations::values()
{
    if (canProceed()) {
        QModelIndex index = devicesLV->selectionModel()->selectedIndexes().first();
        kDebug() << index.data(DevicesModel::DeviceURI).toString();
        m_args["device-uri"] = index.data(DevicesModel::DeviceURI).toString();
        m_args["device-make-and-model"] = index.data(DevicesModel::DeviceMakeAndModel).toString();
        m_args["device-info"] = index.data(DevicesModel::DeviceInfo).toString();
    }
    return m_args;
}

bool PageDestinations::canProceed()
{
    // It can proceed if one and JUST one item is selected
    // (if the user clicks on the category all items in it get selected)
    return (!devicesLV->selectionModel()->selectedIndexes().isEmpty() &&
             devicesLV->selectionModel()->selectedIndexes().size() == 1);
}

void PageDestinations::checkSelected()
{
    emit allowProceed(canProceed());
}

#include "PageDestinations.moc"
