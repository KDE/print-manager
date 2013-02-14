/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
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

#include "PageDestinations.h"
#include "ui_PageDestinations.h"

#include "DevicesModel.h"

#include "ChooseLpd.h"
#include "ChooseSamba.h"
#include "ChooseSerial.h"
#include "ChooseSocket.h"
#include "ChooseUri.h"

#include <KCupsRequest.h>
#include <NoSelectionRectDelegate.h>

#include <QItemSelectionModel>
#include <QStringBuilder>

#include <KDebug>

// system-config-printer --setup-printer='file:/tmp/printout' --devid='MFG:Ricoh;MDL:Aficio SP C820DN'
PageDestinations::PageDestinations(const QVariantHash &args, QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageDestinations),
    m_chooseLpd(new ChooseLpd(this)),
    m_chooseSamba(new ChooseSamba(this)),
    m_chooseSerial(new ChooseSerial(this)),
    m_chooseSocket(new ChooseSocket(this)),
    m_chooseUri(new ChooseUri(this)),
    m_chooseLabel(new QLabel(this))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->stackedWidget->addWidget(m_chooseLpd);
    connect(m_chooseLpd, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));
    connect(m_chooseLpd, SIGNAL(startWorking()), SLOT(working()));
    connect(m_chooseLpd, SIGNAL(stopWorking()), SLOT(notWorking()));

    ui->stackedWidget->addWidget(m_chooseSamba);
    connect(m_chooseSamba, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));
    connect(m_chooseSamba, SIGNAL(startWorking()), SLOT(working()));
    connect(m_chooseSamba, SIGNAL(stopWorking()), SLOT(notWorking()));

    ui->stackedWidget->addWidget(m_chooseSerial);
    connect(m_chooseSerial, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));
    connect(m_chooseSerial, SIGNAL(startWorking()), SLOT(working()));
    connect(m_chooseSerial, SIGNAL(stopWorking()), SLOT(notWorking()));

    ui->stackedWidget->addWidget(m_chooseSocket);
    connect(m_chooseSocket, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));
    connect(m_chooseSocket, SIGNAL(startWorking()), SLOT(working()));
    connect(m_chooseSocket, SIGNAL(stopWorking()), SLOT(notWorking()));

    ui->stackedWidget->addWidget(m_chooseUri);
    connect(m_chooseUri, SIGNAL(allowProceed(bool)), SIGNAL(allowProceed(bool)));
    connect(m_chooseUri, SIGNAL(startWorking()), SLOT(working()));
    connect(m_chooseUri, SIGNAL(stopWorking()), SLOT(notWorking()));
    connect(m_chooseUri, SIGNAL(errorMessage(QString)), ui->messageWidget, SLOT(setText(QString)));
    connect(m_chooseUri, SIGNAL(errorMessage(QString)), ui->messageWidget, SLOT(animatedShow()));
    connect(m_chooseUri, SIGNAL(insertDevice(QString,QString,QString,QString,QString,QString,KCupsPrinters)),
            SLOT(insertDevice(QString,QString,QString,QString,QString,QString,KCupsPrinters)));

    m_chooseLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->stackedWidget->addWidget(m_chooseLabel);

    // Hide the message widget
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->hide();

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
    m_model = new DevicesModel(this);
    ui->devicesTV->setModel(m_model);
    ui->devicesTV->setItemDelegate(new NoSelectionRectDelegate(this));
    connect(ui->devicesTV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(deviceChanged()));
    connect(m_model, SIGNAL(errorMessage(QString)), ui->messageWidget, SLOT(setText(QString)));
    connect(m_model, SIGNAL(errorMessage(QString)), ui->messageWidget, SLOT(animatedShow()));

    // Expand when a parent is added
    connect(m_model, SIGNAL(parentAdded(QModelIndex)),
            ui->devicesTV, SLOT(expand(QModelIndex)));

    // Update the view when the device URI combo box changed
    connect(ui->connectionsCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(deviceUriChanged()));
    ui->connectionsGB->setVisible(false);

    // Setup the busy cursor
    working();
    connect(m_model, SIGNAL(loaded()), this, SLOT(notWorking()));

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
    bool ret = ui->stackedWidget->currentIndex() != 0;

    GenericPage *page = qobject_cast<GenericPage*>(ui->stackedWidget->currentWidget());
    if (page) {
        ret = page->canProceed();
    }

    return ret;
}

void PageDestinations::deviceChanged()
{
    QItemSelectionModel *selection = ui->devicesTV->selectionModel();
    if (!selection->selectedIndexes().isEmpty() &&
            selection->selectedIndexes().size() == 1) {
        QModelIndex index = selection->selectedIndexes().first();
        QVariant uris = index.data(DevicesModel::DeviceUris);
        if (uris.isNull()) {
            ui->connectionsGB->setVisible(false);
        } else if (uris.type() == QVariant::StringList) {
            ui->connectionsCB->clear();
            foreach (const QString &uri, uris.toStringList()) {
                ui->connectionsCB->addItem(uriText(uri), uri);
            }
            ui->connectionsGB->setVisible(true);
        } else {
            ui->connectionsCB->clear();
            foreach (const KCupsPrinter &printer, uris.value<KCupsPrinters>()) {
                ui->connectionsCB->addItem(printer.name(), qVariantFromValue(printer));
            }
            ui->connectionsGB->setVisible(true);
        }
    } else {
        ui->connectionsGB->setVisible(false);
        setCurrentPage(0, selectedItemValues());
        return;
    }

    deviceUriChanged();
}

void PageDestinations::deviceUriChanged()
{
    // Get the selected values
    QVariantHash args = selectedItemValues();

    // "beh" is excluded from the list
    QString deviceUri = args[KCUPS_DEVICE_URI].toString();
    kDebug() << deviceUri;
    if (deviceUri.startsWith(QLatin1String("parallel"))) {
        m_chooseLabel->setText(i18n("A printer connected to the parallel port."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("usb"))) {
        m_chooseLabel->setText(i18n("A printer connected to a USB port."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("bluetooth"))) {
        m_chooseLabel->setText(i18n("A printer connected via Bluetooth."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("hal"))) {
        m_chooseLabel->setText(i18n("Local printer detected by the "
                                    "Hardware Abstraction Layer (HAL)."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("hp"))) {
        m_chooseLabel->setText(i18n("HPLIP software driving a printer, "
                                    "or the printer function of a multi-function device."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("hpfax"))) {
        m_chooseLabel->setText(i18n("HPLIP software driving a fax machine, "
                                    "or the fax function of a multi-function device."));
        setCurrentPage(m_chooseLabel, args);
    } else if (deviceUri.startsWith(QLatin1String("dnssd")) ||
               deviceUri.startsWith(QLatin1String("mdns"))) {
        // TODO this needs testing...
        QString text;
        if (deviceUri.contains(QLatin1String("cups"))) {
            text = i18n("Remote CUPS printer via DNS-SD");
        } else {
            QString protocol;
            if (deviceUri.contains(QLatin1String("._ipp"))) {
                protocol = QLatin1String("IPP");
            } else if (deviceUri.contains(QLatin1String("._printer"))) {
                protocol = QLatin1String("LPD");
            } else if (deviceUri.contains(QLatin1String("._pdl-datastream"))) {
                protocol = QLatin1String("AppSocket/JetDirect");
            }

            if (protocol.isNull()) {
                text = i18n("Network printer via DNS-SD");
            } else {
                text = i18n("%1 network printer via DNS-SD", protocol);
            }
        }
        m_chooseLabel->setText(text);
        setCurrentPage(m_chooseLabel, args);
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

    emit allowProceed(canProceed());
}

void PageDestinations::insertDevice(const QString &device_class, const QString &device_id, const QString &device_info, const QString &device_make_and_model, const QString &device_uri, const QString &device_location, const KCupsPrinters &grouped_printers)
{
    m_model->insertDevice(device_class,
                          device_id,
                          device_info,
                          device_make_and_model,
                          device_uri,
                          device_location,
                          grouped_printers);
}

QVariantHash PageDestinations::selectedItemValues() const
{
    QVariantHash ret = m_args;
    if (!ui->devicesTV->selectionModel()->selectedIndexes().isEmpty() &&
            ui->devicesTV->selectionModel()->selectedIndexes().size() == 1) {
        QModelIndex index = ui->devicesTV->selectionModel()->selectedIndexes().first();
        QVariant uri = index.data(DevicesModel::DeviceUri);
        QVariant uris = index.data(DevicesModel::DeviceUris);
        // if the devicesTV holds an item with grouped URIs
        // get the selected value from the connections combo box
        if (uris.isNull() || uris.type() == QVariant::StringList) {
            if (uris.type() == QVariant::StringList) {
                uri = ui->connectionsCB->itemData(ui->connectionsCB->currentIndex());
            }
            ret[KCUPS_DEVICE_URI] = uri;
            ret[KCUPS_DEVICE_ID] = index.data(DevicesModel::DeviceId);
            ret[KCUPS_DEVICE_MAKE_AND_MODEL] = index.data(DevicesModel::DeviceMakeAndModel);
            ret[KCUPS_DEVICE_INFO] = index.data(DevicesModel::DeviceInfo);
            ret[KCUPS_DEVICE_LOCATION] = index.data(DevicesModel::DeviceLocation);
        } else {
            QVariant aux = ui->connectionsCB->itemData(ui->connectionsCB->currentIndex());
            KCupsPrinter printer = aux.value<KCupsPrinter>();
            KUrl url = uri.toString();
            url.setPath(QLatin1String("printers/") % printer.name());
            ret[KCUPS_DEVICE_URI] = url.url();
            ret[KCUPS_DEVICE_ID] = index.data(DevicesModel::DeviceId);
            ret[KCUPS_PRINTER_INFO] = printer.info();
            kDebug() << KCUPS_PRINTER_INFO << printer.info();
            ret[KCUPS_PRINTER_NAME] = printer.name();
            ret[KCUPS_DEVICE_LOCATION] = printer.location();
        }
        kDebug() << uri << ret;
    }
    return ret;
}

void PageDestinations::setCurrentPage(QWidget *widget, const QVariantHash &args)
{
    GenericPage *page = qobject_cast<GenericPage*>(widget);
    if (page) {
        page->setValues(args);
        if (ui->stackedWidget->currentWidget() != page) {;
            ui->stackedWidget->setCurrentWidget(page);
        }
    } else if (qobject_cast<QLabel*>(widget)) {
        if (ui->connectionsGB->isVisible() &&
                ui->connectionsCB->currentText() == m_chooseLabel->text()) {
            // Don't show duplicated text for the user
            m_chooseLabel->clear();
        }

        if (ui->stackedWidget->currentWidget() != widget) {;
            ui->stackedWidget->setCurrentWidget(widget);
        }
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

QString PageDestinations::uriText(const QString &uri) const
{
    QString ret;
    if (uri.startsWith(QLatin1String("parallel"))) {
        ret = i18n("Parallel Port");
    } else if (uri.startsWith(QLatin1String("serial"))) {
        ret = i18n("Serial Port");
    } else if (uri.startsWith(QLatin1String("usb"))) {
        ret = i18n("USB");
    } else if (uri.startsWith(QLatin1String("bluetooth")) ){
        ret = i18n("Bluetooth");
    } else if (uri.startsWith(QLatin1String("hpfax"))) {
        ret = i18n("Fax - HP Linux Imaging and Printing (HPLIP)");
    } else if (uri.startsWith(QLatin1String("hp"))) {
        ret = i18n("HP Linux Imaging and Printing (HPLIP)");
    } else if (uri.startsWith(QLatin1String("hal"))) {
        ret = i18n("Hardware Abstraction Layer (HAL)");
    } else if (uri.startsWith(QLatin1String("socket"))) {
        ret = i18n("AppSocket/HP JetDirect");
    } else if (uri.startsWith(QLatin1String("lpd"))) {
        // Check if the queue name is defined
        QString queue = uri.section(QLatin1Char('/'), -1, -1);
        if (queue.isEmpty()) {
            ret = i18n("LPD/LPR queue");
        } else {
            ret = i18n("LPD/LPR queue %1", queue);
        }
    } else if (uri.startsWith(QLatin1String("smb"))) {
        ret = i18n("Windows Printer via SAMBA");
    } else if (uri.startsWith(QLatin1String("ipp"))) {
        // Check if the queue name (fileName) is defined
        QString queue = uri.section(QLatin1Char('/'), -1, -1);
        if (queue.isEmpty()) {
            ret = i18n("IPP");
        } else {
            ret = i18n("IPP %1", queue);
        }
    } else if (uri.startsWith(QLatin1String("https"))) {
        ret = i18n("HTTP");
    } else if (uri.startsWith(QLatin1String("dnssd")) ||
               uri.startsWith(QLatin1String("mdns"))) {
        // TODO this needs testing...
        QString text;
        if (uri.contains(QLatin1String("cups"))) {
            text = i18n("Remote CUPS printer via DNS-SD");
        } else {
            if (uri.contains(QLatin1String("._ipp"))) {
                ret = i18n("IPP network printer via DNS-SD");
            } else if (uri.contains(QLatin1String("._printer"))) {
                ret = i18n("LPD network printer via DNS-SD");
            } else if (uri.contains(QLatin1String("._pdl-datastream"))) {
                ret = i18n("AppSocket/JetDirect network printer via DNS-SD");
            } else {
                ret = i18n("Network printer via DNS-SD");
            }
        }
    } else {
        ret = uri;
    }
    return ret;
}
