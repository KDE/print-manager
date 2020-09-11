/***************************************************************************
 *   Copyright (C) 2010-2018 by Daniel Nicoletti                           *
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

#include "PageAddPrinter.h"
#include "ui_PageAddPrinter.h"

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QPainter>
#include <QPointer>
#include <QRegExpValidator>
#include <QDebug>

PageAddPrinter::PageAddPrinter(QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageAddPrinter)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    const int printerSize = 128;
    const int overlaySize = 48;

    QPixmap printerIcon = QIcon::fromTheme(QStringLiteral("printer")).pixmap(printerSize);
    const QPixmap preferencesIcon = QIcon::fromTheme(QStringLiteral("dialog-information")).pixmap(overlaySize);

    QPainter painter(&printerIcon);

    // bottom right corner
    const QPoint startPoint = QPoint(printerSize - overlaySize - 2,
                                     printerSize - overlaySize - 2);
    painter.drawPixmap(startPoint, preferencesIcon);

    ui->printerL->setPixmap(printerIcon);

    // May contain any printable characters except "/", "#", and space
    QRegExp rx(QLatin1String("[^/#\\ ]*"));
    auto validator = new QRegExpValidator(rx, this);
    ui->nameLE->setValidator(validator);

    // Hide the message widget
    ui->messageWidget->setWordWrap(true);
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->hide();
}

PageAddPrinter::~PageAddPrinter()
{
    delete ui;
}

void PageAddPrinter::setValues(const QVariantHash &args)
{
    if (m_args != args) {
        QString name;
        if (!args[KCUPS_PRINTER_NAME].toString().isEmpty()) {
            name = args[KCUPS_PRINTER_NAME].toString();
        } else if (!args[KCUPS_DEVICE_MAKE_AND_MODEL].toString().isEmpty()) {
            name = args[KCUPS_DEVICE_MAKE_AND_MODEL].toString();
        } else if (!args[KCUPS_DEVICE_INFO].toString().isEmpty()) {
            name = args[KCUPS_DEVICE_INFO].toString();
        }

        if (!args[KCUPS_PRINTER_INFO].toString().isEmpty()) {
            ui->descriptionLE->setText(args[KCUPS_PRINTER_INFO].toString());
        } else {
            ui->descriptionLE->setText(name);
        }

        name.replace(QLatin1Char(' '), QLatin1Char('_'));
        name.replace(QLatin1Char('/'), QLatin1Char('-'));
        name.replace(QLatin1Char('#'), QLatin1Char('='));
        ui->nameLE->setText(name);
        ui->locationLE->setText(args[KCUPS_DEVICE_LOCATION].toString());
        ui->shareCB->setChecked(true);
        ui->shareCB->setVisible(args[ADDING_PRINTER].toBool());

        m_args = args;
    }
}

void PageAddPrinter::load()
{
}

bool PageAddPrinter::canProceed() const
{
    return !ui->nameLE->text().isEmpty();
}

bool PageAddPrinter::finishClicked()
{
    bool ret = false;
    QVariantHash args = values();
    args[KCUPS_PRINTER_IS_ACCEPTING_JOBS] = true;
    args[KCUPS_PRINTER_STATE] = IPP_PRINTER_IDLE;

    // Check if it's a printer or a class that we are adding
    bool isClass = !args.take(ADDING_PRINTER).toBool();
    QString destName = args[KCUPS_PRINTER_NAME].toString();
    QString filename = args.take(FILENAME).toString();

    QPointer<KCupsRequest> request = new KCupsRequest;
    if (isClass) {
        request->addOrModifyClass(destName, args);
    } else {
        request->addOrModifyPrinter(destName, args, filename);
    }
    request->waitTillFinished();
    if (request) {
        if (request->hasError()) {
            qDebug() << request->error() << request->errorMsg();
            QString message;
            if (isClass) {
                message = i18nc("@info", "Failed to add class: '%1'", request->errorMsg());
            } else {
                message = i18nc("@info", "Failed to configure printer: '%1'", request->errorMsg());
            }
            ui->messageWidget->setText(message);
            ui->messageWidget->animatedShow();
        } else {
            ret = true;
        }
        request->deleteLater();
    }

    return ret;
}

QVariantHash PageAddPrinter::values() const
{
    QVariantHash ret = m_args;
    ret[KCUPS_PRINTER_NAME] = ui->nameLE->text();
    ret[KCUPS_PRINTER_LOCATION] = ui->locationLE->text();
    ret[KCUPS_PRINTER_INFO] = ui->descriptionLE->text();
    if (ret[ADDING_PRINTER].toBool()) {
         ret[KCUPS_PRINTER_IS_SHARED] = ui->shareCB->isChecked();
    }
    return ret;
}

void PageAddPrinter::on_nameLE_textChanged(const QString &text)
{
    emit allowProceed(!text.isEmpty());
}

void PageAddPrinter::checkSelected()
{
//     emit allowProceed(!devicesLV->selectionModel()->selection().isEmpty());
}

#include "moc_PageAddPrinter.cpp"
