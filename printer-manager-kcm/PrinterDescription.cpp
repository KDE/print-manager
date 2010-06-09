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

#include <SupplyLevels.h>

#include <QPainter>
#include <QPointer>
#include <QDBusMessage>

#include <QDBusConnection>
#include <KMenu>
#include <KDebug>

#define PRINTER_ICON_SIZE 128

PrinterDescription::PrinterDescription(QWidget *parent)
 : QWidget(parent),
   m_isClass(false),
   m_markerChangeTime(0)
{
    setupUi(this);

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

    KMenu *menu = new KMenu(maintenancePB);
    menu->addAction(actionPrintTestPage);
    menu->addAction(actionCleanPrintHeads);
    menu->addAction(actionPrintSelfTestPage);
    actionCleanPrintHeads->setVisible(false);
    actionPrintSelfTestPage->setVisible(false);
    maintenancePB->setMenu(menu);
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
    bool isDefault = defaultCB->isChecked();
    QCups::Result *ret = QCups::setDefaultPrinter(m_destName);
    ret->waitTillFinished();
    setIsDefault(ret->hasError() ? !isDefault : isDefault);
    ret->deleteLater();
}

void PrinterDescription::on_sharedCB_clicked()
{
    bool shared = sharedCB->isChecked();
    QCups::Result *ret = QCups::Dest::setShared(m_destName, m_isClass, shared);
    ret->waitTillFinished();
    setIsShared(ret->hasError() ? !shared : shared);
    ret->deleteLater();
}

void PrinterDescription::on_supplyLevelsPB_clicked()
{
    QCups::SupplyLevels *dialog = new QCups::SupplyLevels(m_markerData, this);
    dialog->exec();
}

void PrinterDescription::setPrinterIcon(const QIcon &icon)
{
    iconL->setPixmap(icon.pixmap(PRINTER_ICON_SIZE, PRINTER_ICON_SIZE));
}

void PrinterDescription::setDestName(const QString &name, const QString &description, bool isClass)
{
    m_destName = name;

    m_markerData.clear();
    if (m_isClass != isClass) {
        m_isClass = isClass;
        sharedCB->setText(m_isClass ? i18n("Share this class") : i18n("Share this printer"));
    }

    if (!description.isEmpty() && description != printerNameL->text()) {
        printerNameL->setText(description);
    } else if (description.isEmpty() && name != printerNameL->text()) {
        printerNameL->setText(name);
    }
}

void PrinterDescription::setLocation(const QString &location)
{
    locationMsgL->setText(location);
}

void PrinterDescription::setStatus(const QString &status)
{
    statusMsgL->setText(status);
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

void PrinterDescription::setCommands(const QStringList &commands)
{
    if (m_commands != commands) {
        m_commands = commands;

        actionCleanPrintHeads->setVisible(commands.contains("Clean"));
        actionPrintSelfTestPage->setVisible(commands.contains("PrintSelfTestPage"));
        supplyLevelsPB->setEnabled(commands.contains("ReportLevels"));
    }
}

void PrinterDescription::setMarkerLevels(const QVariant &data)
{
    m_markerData["marker-levels"] = data;
}

void PrinterDescription::setMarkerColors(const QVariant &data)
{
    m_markerData["marker-colors"] = data;
}

void PrinterDescription::setMarkerNames(const QVariant &data)
{
    m_markerData["marker-names"] = data;
}

void PrinterDescription::setMarkerTypes(const QVariant &data)
{
    m_markerData["marker-types"] = data;
}

bool PrinterDescription::needMarkerLevels(int markerChangeTime)
{
    return m_markerChangeTime != markerChangeTime;
}

void PrinterDescription::on_actionPrintTestPage_triggered(bool checked)
{
    Q_UNUSED(checked)
    // TODO Show a msg box if failed
    QCups::Result *ret = QCups::Dest::printTestPage(m_destName, m_isClass);
    ret->waitTillFinished();
    ret->deleteLater();
}

void PrinterDescription::on_actionCleanPrintHeads_triggered(bool checked)
{
    Q_UNUSED(checked)
    QCups::Result *ret = QCups::Dest::printCommand(m_destName, "Clean all", i18n("Clean Print Heads"));
    ret->waitTillFinished();
    ret->deleteLater();
}

void PrinterDescription::on_actionPrintSelfTestPage_triggered(bool checked)
{
    Q_UNUSED(checked)
    QCups::Result *ret = QCups::Dest::printCommand(m_destName, "PrintSelfTestPage", i18n("Print Self-Test Page"));
    ret->waitTillFinished();
    ret->deleteLater();
}

QString PrinterDescription::destName() const
{
    return m_destName;
}


#include "PrinterDescription.moc"
