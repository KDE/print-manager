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

#include "PrinterDescription.h"

#include "ConfigureDialog.h"

#include <QCups.h>
#include <cups/cups.h>

#include <QPainter>
#include <QPointer>
#include <QDBusMessage>

#include <QDBusConnection>
#include <KDebug>

#define PRINTER_ICON_SIZE 128

PrinterDescription::PrinterDescription(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

//     // setup default options
    // loads the standard key icon
    m_printerIcon = KIconLoader::global()->loadIcon("printer",
                                                    KIconLoader::NoGroup,
                                                    PRINTER_ICON_SIZE, // a not so huge icon
                                                    KIconLoader::DefaultState);
    iconL->setPixmap(m_printerIcon);

    m_pauseIcon = KIconLoader::global()->loadIcon("media-playback-pause",
                                                  KIconLoader::NoGroup,
                                                  KIconLoader::SizeMedium,
                                                  KIconLoader::DefaultState,
                                                  QStringList(),
                                                  0,
                                                  true);
}

PrinterDescription::~PrinterDescription()
{
}

void PrinterDescription::on_openQueuePB_clicked()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.PrintQueue",
                                             "/",
                                             "org.kde.PrintQueue",
                                             QLatin1String("ShowQueue"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(m_destName);
    QDBusConnection::sessionBus().call(message);
}

void PrinterDescription::on_defaultCB_clicked()
{
    setIsDefault(QCups::setDefaultPrinter(m_destName));
}

void PrinterDescription::on_sharedCB_clicked()
{
    setIsShared(QCups::Printer::setShared(m_destName, sharedCB->isChecked()));
}

void PrinterDescription::on_optionsPB_clicked()
{
    QPointer<QCups::ConfigureDialog> dlg = new QCups::ConfigureDialog(m_destName, this);
    dlg->exec();
    delete dlg;
}

void PrinterDescription::setDestName(const QString &name)
{
    m_destName = name;
    printerNameL->setText(name);
}

void PrinterDescription::setLocation(const QString &location)
{
    locationMsgL->setText(location);
}

void PrinterDescription::setStatus(const QString &status)
{
    statusMsgL->setText(status);
}

void PrinterDescription::setDescription(const QString &description)
{
    descriptionL->setText(description);
}

void PrinterDescription::setKind(const QString &kind)
{
    kindMsgL->setText(kind);
}

void PrinterDescription::setIsDefault(bool isDefault)
{
    defaultCB->setEnabled(!isDefault);
    defaultCB->setChecked(isDefault);
}

void PrinterDescription::setIsShared(bool isShared)
{
    sharedCB->setChecked(isShared);
}

#include "PrinterDescription.moc"
