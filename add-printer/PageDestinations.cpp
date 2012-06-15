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

#include "ChooseLpd.h"
#include "ChoosePrinters.h"
#include "ChooseSamba.h"
#include "ChooseSerial.h"
#include "ChooseSocket.h"
#include "ChooseUri.h"

#include <QPainter>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KDebug>
#include <KPixmapSequence>

// system-config-printer --setup-printer='file:/tmp/printout' --devid='MFG:Ricoh;MDL:Aficio SP C820DN'
PageDestinations::PageDestinations(const QVariantHash &args, QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageDestinations),
    m_canProceed(false),
    m_chooseLpd(new ChooseLpd(this)),
    m_choosePrinters(new ChoosePrinters(this)),
    m_chooseSamba(new ChooseSamba(this)),
    m_chooseSerial(new ChooseSerial(this)),
    m_chooseSocket(new ChooseSocket(this)),
    m_chooseUri(new ChooseUri(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->stackedWidget->addWidget(m_chooseLpd);
    connect(m_chooseLpd, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));

    ui->stackedWidget->addWidget(m_choosePrinters);
    connect(m_choosePrinters, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));

    ui->stackedWidget->addWidget(m_chooseSamba);
    connect(m_chooseSamba, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));

    ui->stackedWidget->addWidget(m_chooseSerial);
    connect(m_chooseSerial, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));

    ui->stackedWidget->addWidget(m_chooseSocket);
    connect(m_chooseSocket, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));

    ui->stackedWidget->addWidget(m_chooseUri);
    connect(m_chooseUri, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));

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

//    pixmap = KIconLoader::global()->loadIcon("page-zoom",
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

    m_model = new DevicesModel(this);
    ui->devicesTV->setModel(m_model);
    connect(ui->devicesTV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(checkSelected()));

    // Expand when a parent is added
    connect(m_model, SIGNAL(parentAdded(QModelIndex)),
            ui->devicesTV, SLOT(expand(QModelIndex)));

    // Setup the busy cursor
//    m_busySeq = new KPixmapSequenceOverlayPainter(this);
//    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
//    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
//    m_busySeq->setWidget(ui->printerL);
//    connect(m_model, SIGNAL(loaded()), m_busySeq, SLOT(stop()));

    if (!args.isEmpty()) {
        // set our args
        setValues(args);
    }
}

PageDestinations::~PageDestinations()
{
    delete ui;
}

void PageDestinations::setValues(const QVariantHash &args)
{
    m_args = args;
    if (args[ADDING_PRINTER].toBool()) {
//        m_isValid = true;
        m_model->update();
//        m_busySeq->start();
    } else {
//        m_isValid = false;
    }
}

bool PageDestinations::isValid() const
{
    return true;
}

bool PageDestinations::hasChanges() const
{
    if (!isValid()) {
        return false;
    }

    QString deviceURI;
    if (canProceed()) {
        deviceURI = ui->devicesTV->selectionModel()->selectedIndexes().first().data(DevicesModel::DeviceUri).toString();
    }
    return deviceURI != m_args[DEVICE_URI];
}

QVariantHash PageDestinations::values() const
{
    QVariantHash ret = m_args;
    GenericPage *page = qobject_cast<GenericPage*>(ui->stackedWidget->currentWidget());
    if (page) {
        ret = page->values();
    } else if (canProceed()) {
        ret = selectedItemValues();
    }
    return ret;
}

bool PageDestinations::canProceed() const
{
    bool ret = m_canProceed;

    GenericPage *page = qobject_cast<GenericPage*>(ui->stackedWidget->currentWidget());
    if (page) {
        ret = page->canProceed();
    }

    return ret;
}

void PageDestinations::checkSelected()
{
    if (!ui->devicesTV->selectionModel()->selectedIndexes().isEmpty() &&
            ui->devicesTV->selectionModel()->selectedIndexes().size() == 1) {
        // Get the selected index
        QVariantHash args = selectedItemValues();
        QString deviceUri = args[DEVICE_URI].toString();
        if (deviceUri.startsWith(QLatin1String("parallel")) ||
                deviceUri.startsWith(QLatin1String("usb")) ||
                deviceUri.startsWith(QLatin1String("bluetooth")) ||
                deviceUri.startsWith(QLatin1String("hal")) ||
                deviceUri.startsWith(QLatin1String("beh")) ||
                deviceUri.startsWith(QLatin1String("hp")) ||
                deviceUri.startsWith(QLatin1String("hpfax")) ||
                deviceUri.startsWith(QLatin1String("dnssd"))) {
            // Set as true to enable the Next buttom
            m_canProceed = true;
        } else if (deviceUri.startsWith(QLatin1String("socket"))) {
            kDebug() << "SOCKET";
            setCurrentPage(m_chooseSocket, args);
        } else if (deviceUri.startsWith(QLatin1String("ipp")) ||
                   deviceUri.startsWith(QLatin1String("ipps")) ||
                   deviceUri.startsWith(QLatin1String("http")) ||
                   deviceUri.startsWith(QLatin1String("https"))) {
            setCurrentPage(m_chooseUri, args);
        } else if (deviceUri.startsWith(QLatin1String("lpd"))) {
            setCurrentPage(m_chooseLpd, args);
        } else if (deviceUri.startsWith(QLatin1String("scsi"))) {
            // TODO
            setCurrentPage(m_chooseUri, args);
        } else if (deviceUri.startsWith(QLatin1String("serial"))) {
            setCurrentPage(m_chooseSerial, args);
        } else if (deviceUri.startsWith(QLatin1String("smb"))) {
            setCurrentPage(m_chooseSamba, args);
        } else if (deviceUri.startsWith(QLatin1String("network"))) {
            setCurrentPage(m_chooseUri, args);
        } else {
            setCurrentPage(m_chooseUri, args);
        }
    } else {
        m_canProceed = false;
        setCurrentPage(0, selectedItemValues());
    }

    emit allowProceed(canProceed());
}

QVariantHash PageDestinations::selectedItemValues() const
{
    QVariantHash ret = m_args;
    if (!ui->devicesTV->selectionModel()->selectedIndexes().isEmpty() &&
            ui->devicesTV->selectionModel()->selectedIndexes().size() == 1) {
        QModelIndex index = ui->devicesTV->selectionModel()->selectedIndexes().first();
        ret[DEVICE_URI]        = index.data(DevicesModel::DeviceUri);
        ret[DEVICE_ID]         = index.data(DevicesModel::DeviceId);
        ret[DEVICE_MAKE_MODEL] = index.data(DevicesModel::DeviceMakeAndModel);
        ret[DEVICE_INFO]       = index.data(DevicesModel::DeviceInfo);
        ret[DEVICE_LOCATION]   = index.data(DevicesModel::DeviceLocation);
    }
    return ret;
}

void PageDestinations::setCurrentPage(GenericPage *page, const QVariantHash &args)
{
    if (page) {
        page->setValues(args);
        if (ui->stackedWidget->currentWidget() != page) {;
            ui->stackedWidget->setCurrentWidget(page);
        }
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}
