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

#include "PageChoose.h"

#include "ChooseIpp.h"
#include "ChooseLpd.h"
#include "ChoosePrinters.h"
#include "ChooseSamba.h"
#include "ChooseSerial.h"
#include "ChooseSocket.h"
#include "ChooseUri.h"

#include <KDebug>

PageChoose::PageChoose(const QVariantHash &args, QWidget *parent) :
    GenericPage(parent)
{
    m_chooseIpp      = new ChooseIpp(this);
    m_chooseLpd      = new ChooseLpd(this);
    m_choosePrinters = new ChoosePrinters(this);
    m_chooseSamba    = new ChooseSamba(this);
    m_chooseSerial   = new ChooseSerial(this);
    m_chooseSocket   = new ChooseSocket(this);
    m_chooseUri      = new ChooseUri(this);

    m_layout = new QStackedLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_chooseIpp);
    m_layout->addWidget(m_chooseLpd);
    m_layout->addWidget(m_choosePrinters);
    m_layout->addWidget(m_chooseSamba);
    m_layout->addWidget(m_chooseSerial);
    m_layout->addWidget(m_chooseSocket);
    m_layout->addWidget(m_chooseUri);

    setLayout(m_layout);

    // Set our args
    if (!args.isEmpty()) {
        setValues(args);
    }
}

PageChoose::~PageChoose()
{
}

void PageChoose::setValues(const QVariantHash &args)
{
    kDebug() << args;
    if (m_args == args) {
        return;
    }

    // Disconnect old SIGNAL
    GenericPage *oldPage = qobject_cast<GenericPage *>(m_layout->currentWidget());
    disconnect(oldPage, SIGNAL(allowProceed(bool)), this, SIGNAL(allowProceed(bool)));

    m_isValid = true;
    m_args = args;
    if (args["add-new-printer"].toBool()) {
        QString deviceUri = args["device-uri"].toString();
        if (deviceUri.startsWith(QLatin1String("parallel")) ||
            deviceUri.startsWith(QLatin1String("usb")) ||
            deviceUri.startsWith(QLatin1String("bluetooth")) ||
            deviceUri.startsWith(QLatin1String("hal")) ||
            deviceUri.startsWith(QLatin1String("beh")) ||
            deviceUri.startsWith(QLatin1String("hp")) ||
            deviceUri.startsWith(QLatin1String("hpfax")) ||
            deviceUri.startsWith(QLatin1String("dnssd"))) {
            // Set as false to jump to the next page
            m_isValid = false;
        } else if (deviceUri.startsWith(QLatin1String("socket"))) {
                kDebug() << "SOCKET";
            m_layout->setCurrentWidget(m_chooseSocket);
        } else if (deviceUri.startsWith(QLatin1String("ipp")) ||
                   deviceUri.startsWith(QLatin1String("http")) ||
                   deviceUri.startsWith(QLatin1String("https"))) {
            m_layout->setCurrentWidget(m_chooseIpp);
        } else if (deviceUri.startsWith(QLatin1String("lpd"))) {
            m_layout->setCurrentWidget(m_chooseLpd);
        } else if (deviceUri.startsWith(QLatin1String("scsi"))) {
            // TODO
            m_layout->setCurrentWidget(m_chooseUri);
        } else if (deviceUri.startsWith(QLatin1String("serial"))) {
            m_layout->setCurrentWidget(m_chooseSerial);
        } else if (deviceUri.startsWith(QLatin1String("smb"))) {
            m_layout->setCurrentWidget(m_chooseSamba);
        } else if (deviceUri.startsWith(QLatin1String("network"))) {
            m_layout->setCurrentWidget(m_chooseUri);
        } else {
            m_layout->setCurrentWidget(m_chooseUri);
        }
    } else {
        kDebug() << "adding a class" << args;
        // we are adding a class
        m_layout->setCurrentWidget(m_choosePrinters);
    }
    GenericPage *page = qobject_cast<GenericPage *>(m_layout->currentWidget());
    page->setValues(args);
    emit allowProceed(page->canProceed());
    connect(page, SIGNAL(allowProceed(bool)), this, SIGNAL(allowProceed(bool)));
}

bool PageChoose::isValid() const
{
    // If we have a valid widget return that the page is valid
    return m_isValid;
}

void PageChoose::load()
{
}

QVariantHash PageChoose::values() const
{
    if (m_isValid) {
        return qobject_cast<GenericPage *>(m_layout->currentWidget())->values();
    } else {
        return m_args;
    }
}

bool PageChoose::canProceed() const
{
    if (m_isValid) {
        return qobject_cast<GenericPage *>(m_layout->currentWidget())->canProceed();
    } else {
        return false;
    }
}

void PageChoose::checkSelected()
{
//     emit allowProceed(!devicesLV->selectionModel()->selection().isEmpty());
}
