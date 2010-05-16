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

#include "PageSerial.h"

#include "DevicesModel.h"

#include <QPainter>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KDebug>

PageSerial::PageSerial(QWidget *parent)
 : GenericPage(parent), m_rx("?baud=(\\d+)"), m_isValid(false)
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
    printerL->setPixmap(icon);

    parityCB->addItem(i18n("None"), "none");
    parityCB->addItem(i18n("Even"), "even");
    parityCB->addItem(i18n("Odd"),  "odd");

    flowCB->addItem(i18n("None"), "none");
    flowCB->addItem(i18n("XON/XOFF (Software)"), "soft");
    flowCB->addItem(i18n("RTS/CTS (Hardware)"),  "hard");
    flowCB->addItem(i18n("DTR/DSR (Hardware)"),  "dtrdsr");
}

PageSerial::~PageSerial()
{
}

bool PageSerial::isValid()
{
    return m_isValid;
};

void PageSerial::setValues(const QHash<QString, QString> &args)
{
    m_args = args;
    if (!args["device-uri"].startsWith(QLatin1String("serial:"))) {
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

    int maxrate;
    if (m_rx.indexIn(args["device-uri"]) != -1) {
        maxrate = m_rx.cap(1).toInt();
    } else {
        maxrate = 19200;
    }

    baudRateCB->clear();
    for (int i = 0; i < 10; i ++) {
        if (baudrates[i] > maxrate) {
            break;
        } else {
            baudRateCB->addItem(QString::number(baudrates[i]));
        }
    }
}

void PageSerial::load()
{
}

QHash<QString, QString> PageSerial::values()
{
    QString deviceUri = m_args["device-uri"];
    int pos = deviceUri.indexOf('?');
    QString baudRate = baudRateCB->currentText();
    QString bits = bitsCB->currentText();
    QString parity = baudRateCB->itemData(baudRateCB->currentIndex()).toString();
    QString flow = flowCB->itemData(flowCB->currentIndex()).toString();
    QString replace = QString("?baud=%1+bits=%2+parity=%3+flow=%4").arg(baudRate).arg(bits).arg(parity).arg(flow);
    deviceUri.replace(pos, deviceUri.size() - pos, replace);
    m_args["device-uri"] = deviceUri;
    return m_args;
}

#include "PageSerial.moc"
