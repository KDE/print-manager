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

#include "ui_PrinterDescription.h"

#include <KCupsRequestPrinters.h>
#include <KCupsPrinter.h>
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
   ui(new Ui::PrinterDescription),
   m_isClass(false),
   m_markerChangeTime(0)
{
    ui->setupUi(this);

    // loads the standard key icon
    m_printerIcon = KIconLoader::global()->loadIcon("printer",
                                                    KIconLoader::NoGroup,
                                                    PRINTER_ICON_SIZE, // a not so huge icon
                                                    KIconLoader::DefaultState);
    ui->iconL->setPixmap(m_printerIcon);

    m_pauseIcon = KIconLoader::global()->loadIcon("media-playback-pause",
                                                  KIconLoader::NoGroup,
                                                  KIconLoader::SizeMedium,
                                                  KIconLoader::DefaultState,
                                                  QStringList(),
                                                  0,
                                                  true);

    KMenu *menu = new KMenu(ui->maintenancePB);
    menu->addAction(ui->actionPrintTestPage);
    menu->addAction(ui->actionCleanPrintHeads);
    menu->addAction(ui->actionPrintSelfTestPage);
    ui->actionCleanPrintHeads->setVisible(false);
    ui->actionPrintSelfTestPage->setVisible(false);
    ui->supplyLevelsPB->setEnabled(false);
    ui->maintenancePB->setMenu(menu);
}

PrinterDescription::~PrinterDescription()
{
    delete ui;
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
    QDBusConnection::sessionBus().send(message);
}

void PrinterDescription::on_defaultCB_clicked()
{
    bool isDefault = ui->defaultCB->isChecked();
    KCupsRequestPrinters *request = new KCupsRequestPrinters;
    request->setDefaultPrinter(m_destName);
    request->waitTillFinished();
    setIsDefault(request->hasError() ? !isDefault : isDefault);
    request->deleteLater();
}

void PrinterDescription::on_sharedCB_clicked()
{
    bool shared = ui->sharedCB->isChecked();
    KCupsRequestPrinters *request = new KCupsRequestPrinters;
    request->setShared(m_destName, m_isClass, shared);
    request->waitTillFinished();
    setIsShared(request->hasError() ? !shared : shared);
    request->deleteLater();
}

void PrinterDescription::on_supplyLevelsPB_clicked()
{
    QCups::SupplyLevels *dialog = new QCups::SupplyLevels(m_markerData, this);
    dialog->exec();
}

void PrinterDescription::setPrinterIcon(const QIcon &icon)
{
    ui->iconL->setPixmap(icon.pixmap(PRINTER_ICON_SIZE, PRINTER_ICON_SIZE));
}

void PrinterDescription::setDestName(const QString &name, const QString &description, bool isClass)
{
    m_destName = name;

    m_markerData.clear();
    if (m_isClass != isClass) {
        m_isClass = isClass;
        ui->sharedCB->setText(m_isClass ? i18n("Share this class") : i18n("Share this printer"));
    }

    if (!description.isEmpty() && description != ui->printerNameL->text()) {
        ui->printerNameL->setText(description);
    } else if (description.isEmpty() && name != ui->printerNameL->text()) {
        ui->printerNameL->setText(name);
    }
}

void PrinterDescription::setLocation(const QString &location)
{
    ui->locationMsgL->setText(location);
}

void PrinterDescription::setStatus(const QString &status)
{
    ui->statusMsgL->setText(status);
}

void PrinterDescription::setKind(const QString &kind)
{
    ui->kindMsgL->setText(kind);
}

void PrinterDescription::setIsDefault(bool isDefault)
{
    ui->defaultCB->setEnabled(!isDefault);
    ui->defaultCB->setChecked(isDefault);
}

void PrinterDescription::setIsShared(bool isShared)
{
    ui->sharedCB->setChecked(isShared);
}

void PrinterDescription::setCommands(const QStringList &commands)
{
    // On the first time this method runs the list
    // can be empty, so keep all objects initilized on the
    // constructor
    if (m_commands != commands) {
        m_commands = commands;

        ui->actionCleanPrintHeads->setVisible(commands.contains("Clean"));
        ui->actionPrintSelfTestPage->setVisible(commands.contains("PrintSelfTestPage"));
        ui->supplyLevelsPB->setEnabled(commands.contains("ReportLevels"));
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
