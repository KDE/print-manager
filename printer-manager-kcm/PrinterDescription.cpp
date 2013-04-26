/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "PrinterDescription.h"

#include "ui_PrinterDescription.h"

#include <KCupsPrinter.h>

#include <QPainter>
#include <QPointer>
#include <QDBusMessage>
#include <QProgressBar>
#include <QLabel>

#include <QDBusConnection>
#include <KMenu>
#include <KDebug>

#define PRINTER_ICON_SIZE 128

Q_DECLARE_METATYPE(QList<int>)

PrinterDescription::PrinterDescription(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrinterDescription),
    m_isClass(false),
    m_markerChangeTime(0)
{
    ui->setupUi(this);
    m_layoutEnd = ui->formLayout->count();

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
    menu->addAction(ui->actionPrintSelfTestPage);
    menu->addAction(ui->actionCleanPrintHeads);
    ui->actionCleanPrintHeads->setVisible(false);
    ui->actionPrintSelfTestPage->setVisible(false);
    ui->maintenancePB->setMenu(menu);
}

PrinterDescription::~PrinterDescription()
{
    delete ui;
}

void PrinterDescription::on_openQueuePB_clicked()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.PrintQueue"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.PrintQueue"),
                                             QLatin1String("ShowQueue"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(m_destName);
    QDBusConnection::sessionBus().send(message);
}

void PrinterDescription::on_defaultCB_clicked()
{
    bool isDefault = ui->defaultCB->isChecked();
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->setDefaultPrinter(m_destName);
    request->waitTillFinished();
    if (request) {
        setIsDefault(request->hasError() ? !isDefault : isDefault);
        request->deleteLater();
    }
}

void PrinterDescription::on_sharedCB_clicked()
{
    bool shared = ui->sharedCB->isChecked();
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->setShared(m_destName, m_isClass, shared);
    request->waitTillFinished();
    if (request) {
        setIsShared(request->hasError() ? !shared : shared);
        request->deleteLater();
    }
}

void PrinterDescription::on_rejectPrintJobsCB_clicked()
{
    bool accepting = !ui->rejectPrintJobsCB->isChecked();
    kDebug() << accepting;
    QPointer<KCupsRequest> request = new KCupsRequest;
    if (accepting) {
        request->acceptJobs(m_destName);
    } else {
        request->rejectJobs(m_destName);

    }
    request->waitTillFinished();
    if (request) {
        setAcceptingJobs(request->hasError() ? !accepting : accepting);
        request->deleteLater();
    }
}

void PrinterDescription::setPrinterIcon(const QIcon &icon)
{
    ui->iconL->setPixmap(icon.pixmap(PRINTER_ICON_SIZE, PRINTER_ICON_SIZE));
}

void PrinterDescription::setDestName(const QString &name, const QString &description, bool isClass, bool singlePrinter)
{
    m_destName = name;

    m_markerData.clear();
    if (m_isClass != isClass) {
        m_isClass = isClass;
        ui->sharedCB->setText(m_isClass ? i18n("Share this class") : i18n("Share this printer"));
    }

    ui->nameMsgL->setText(name);
    if (!description.isEmpty() && description != ui->printerNameL->text()) {
        ui->printerNameL->setText(description);
    } else if (description.isEmpty() && name != ui->printerNameL->text()) {
        ui->printerNameL->setText(name);
    }

    ui->statusL->setVisible(singlePrinter);
    ui->nameL->setVisible(singlePrinter);
    ui->nameMsgL->setVisible(singlePrinter);
}

void PrinterDescription::setDestStatus(const QString &status)
{
    ui->statusL->setText(status);
}

void PrinterDescription::setLocation(const QString &location)
{
    ui->locationMsgL->setText(location);
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
    m_isShared = isShared;
    if (ui->sharedCB->isEnabled()) {
        ui->sharedCB->setChecked(m_isShared);
    }
}

void PrinterDescription::setAcceptingJobs(bool accepting)
{
    ui->rejectPrintJobsCB->setChecked(!accepting);
}

void PrinterDescription::setCommands(const QStringList &commands)
{
    // On the first time this method runs the list
    // can be empty, so keep all objects initialized on the
    // constructor
    if (m_commands != commands) {
        m_commands = commands;

        ui->actionCleanPrintHeads->setVisible(commands.contains("Clean"));
        ui->actionPrintSelfTestPage->setVisible(commands.contains("PrintSelfTestPage"));

        // TODO if the printer supports ReportLevels
        // we should probably probe for them
        // commands.contains("ReportLevels")
    }
}

void PrinterDescription::setMarkers(const QVariantHash &data)
{
    // Remove old progress bars
    while (ui->formLayout->count() > m_layoutEnd) {
        ui->formLayout->takeAt(ui->formLayout->count() - 1)->widget()->deleteLater();
    }

    int size = data["marker-names"].toStringList().size();
    if (size != data["marker-levels"].value<QList<int> >().size() ||
        size != data["marker-colors"].toStringList().size() ||
        size != data["marker-types"].toStringList().size()) {
        return;
    }

    // Create a colored progress bar for each marker
    for (int i = 0; i < size; i++) {
        if (data["marker-types"].toStringList().at(i) == QLatin1String("unknown")) {
            continue;
        }
        QProgressBar *pogressBar = new QProgressBar;
        pogressBar->setValue(data["marker-levels"].value<QList<int> >().at(i));
        pogressBar->setTextVisible(false);
        pogressBar->setMaximumHeight(15);
        QPalette palette = pogressBar->palette();
        palette.setColor(QPalette::Active,
                         QPalette::Highlight,
                         QColor(data["marker-colors"].toStringList().at(i)));
        palette.setColor(QPalette::Inactive,
                         QPalette::Highlight,
                         QColor(data["marker-colors"].toStringList().at(i)).lighter());
        pogressBar->setPalette(palette);
        QLabel *label = new QLabel(data["marker-names"].toStringList().at(i), this);
        ui->formLayout->addRow(label, pogressBar);
    }
}

void PrinterDescription::on_actionPrintTestPage_triggered(bool checked)
{
    Q_UNUSED(checked)
    // TODO Show a msg box if failed

    QPointer<KCupsRequest> request = new KCupsRequest;
    request->printTestPage(m_destName, m_isClass);
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterDescription::on_actionCleanPrintHeads_triggered(bool checked)
{
    Q_UNUSED(checked)
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->printCommand(m_destName, "Clean all", i18n("Clean Print Heads"));
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrinterDescription::on_actionPrintSelfTestPage_triggered(bool checked)
{
    Q_UNUSED(checked)
    QPointer<KCupsRequest> request = new KCupsRequest;
    request->printCommand(m_destName, "PrintSelfTestPage", i18n("Print Self-Test Page"));
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

QString PrinterDescription::destName() const
{
    return m_destName;
}

void PrinterDescription::enableShareCheckBox(bool enable)
{
    ui->sharedCB->setEnabled(enable);
    if (enable) {
        // Get the current printer share state
        ui->sharedCB->setChecked(m_isShared);
    } else {
        ui->sharedCB->setChecked(false);
    }
}

void PrinterDescription::on_configurePB_clicked()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.ConfigurePrinter"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.ConfigurePrinter"),
                                             QLatin1String("ConfigurePrinter"));
    message << m_destName;
    QDBusConnection::sessionBus().call(message);
}
