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

#include "ChooseSerial.h"
#include "ui_ChooseSerial.h"

#include <QPainter>
#include <KDebug>

ChooseSerial::ChooseSerial(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::ChooseSerial),
    m_rx("\\?baud=(\\d+)"),
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

    pixmap = KIconLoader::global()->loadIcon("preferences-other",
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

    ui->parityCB->addItem(i18nc("@label:listbox", "None"), "none");
    ui->parityCB->addItem(i18nc("@label:listbox", "Even"), "even");
    ui->parityCB->addItem(i18nc("@label:listbox", "Odd"),  "odd");

    ui->flowCB->addItem(i18nc("@label:listbox", "None"), "none");
    ui->flowCB->addItem(i18nc("@label:listbox", "XON/XOFF (Software)"), "soft");
    ui->flowCB->addItem(i18nc("@label:listbox", "RTS/CTS (Hardware)"),  "hard");
    ui->flowCB->addItem(i18nc("@label:listbox", "DTR/DSR (Hardware)"),  "dtrdsr");
}

ChooseSerial::~ChooseSerial()
{
    delete ui;
}

bool ChooseSerial::isValid() const
{
    return m_isValid;
};

void ChooseSerial::setValues(const QVariantHash &args)
{
    m_args = args;
    QString deviceUri = args[DEVICE_URI].toString();
    if (!deviceUri.startsWith(QLatin1String("serial:"))) {
        m_isValid = false;
        return;
    }
    m_isValid = true;

    static int    baudrates[] =       /* Baud rates */
    {
        1200,
        2400,
        4800,
        9600,
        19200,
        38400,
        57600,
        115200,
        230400,
        460800
    };

    // Find out the max baud rate
    int maxrate;
    if (m_rx.indexIn(deviceUri) != -1) {
        maxrate = m_rx.cap(1).toInt();
    } else {
        maxrate = 19200;
    }

    ui->baudRateCB->clear();
    for (int i = 0; i < 10; i ++) {
        if (baudrates[i] > maxrate) {
            break;
        } else {
            ui->baudRateCB->addItem(QString::number(baudrates[i]));
        }
    }
    // Set the current index to the maxrate
    ui->baudRateCB->setCurrentIndex(ui->baudRateCB->count() - 1);
}

void ChooseSerial::load()
{
}

QVariantHash ChooseSerial::values() const
{
    QVariantHash ret = m_args;
    QString deviceUri = m_args[DEVICE_URI].toString();
    int pos = deviceUri.indexOf('?');
    QString baudRate = ui->baudRateCB->currentText();
    QString bits = ui->bitsCB->currentText();
    QString parity = ui->baudRateCB->itemData(ui->baudRateCB->currentIndex()).toString();
    QString flow = ui->flowCB->itemData(ui->flowCB->currentIndex()).toString();
    QString replace = QString("?baud=%1+bits=%2+parity=%3+flow=%4").arg(baudRate, bits, parity, flow);
    deviceUri.replace(pos, deviceUri.size() - pos, replace);
    ret[DEVICE_URI] = deviceUri;
    return ret;
}
