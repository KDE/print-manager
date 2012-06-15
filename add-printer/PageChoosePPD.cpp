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
#include "ui_PageChoosePPD.h"

#include "DevicesModel.h"

#include <QPainter>
#include <KDebug>

PageChoosePPD::PageChoosePPD(const QVariantHash &args, QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageChoosePPD),
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
//    ui->printerL->setPixmap(icon);

    m_layout = new QStackedLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    ui->gridLayout->addLayout(m_layout, 1, 3);
    m_selectMM = new SelectMakeModel(this);
    m_layout->addWidget(m_selectMM);

    if (!args.isEmpty()) {
        // set our args
        setValues(args);
    }
}

PageChoosePPD::~PageChoosePPD()
{
    delete ui;
}

void PageChoosePPD::setValues(const QVariantHash &args)
{
    m_args = args;

    if (args[ADDING_PRINTER].toBool()) {
        connect(m_selectMM, SIGNAL(changed(bool)),
                this, SLOT(checkSelected()));
        kDebug() << args;
        m_selectMM->setDeviceInfo(args[DEVICE_ID].toString(),
                                  args[DEVICE_MAKE_MODEL].toString(),
                                  args[DEVICE_URI].toString());
        m_selectMM->setMakeModel(QString(), QString());
        m_isValid = true;
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
    return deviceURI != m_args[DEVICE_URI];
}

QVariantHash PageChoosePPD::values() const
{
    if (!isValid()) {
        return m_args;
    }

    QVariantHash ret = m_args;
    if (canProceed()) {
        // TODO get the PPD file name
        ret[PPD_NAME] = m_selectMM->selectedPPDName();
//        ret[FILENAME] = m_selectMM->selectedPPDFileName();
    }
    return ret;
}

bool PageChoosePPD::canProceed() const
{
    // It can proceed if a PPD file (local or not) is provided    bool changed = false;
    bool allow = false;

    // TODO check the PPD file name
    QString ppdName = m_selectMM->selectedPPDName();
    if (!ppdName.isEmpty()){
        allow = true;
    }

    kDebug() << allow;
    return allow;
}

void PageChoosePPD::checkSelected()
{
    emit allowProceed(canProceed());
}
