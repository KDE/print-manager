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

#include "ChoosePrinters.h"

#include <ClassListWidget.h>

#include <QPainter>
#include <KDebug>

ChoosePrinters::ChoosePrinters(QWidget *parent)
 : GenericPage(parent),
   m_isValid(false)
{
    setupUi(this);

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

    connect(membersLV, SIGNAL(changed(bool)), this, SIGNAL(allowProceed(bool)));
}

ChoosePrinters::~ChoosePrinters()
{
}

bool ChoosePrinters::isValid() const
{
    return m_isValid;
};

void ChoosePrinters::setValues(const QHash<QString, QVariant> &args)
{
    if (m_args != args) {
        membersLV->reload(QString());
        m_args = args;
    }
}

void ChoosePrinters::load()
{
}

QHash<QString, QVariant> ChoosePrinters::values() const
{
    QHash<QString, QVariant> ret = m_args;
    ret["member-uris"] = membersLV->selectedDests();
    return ret;
}

bool ChoosePrinters::canProceed() const
{
    return membersLV->selectedDests().count() > 0;
}

#include "ChoosePrinters.moc"
