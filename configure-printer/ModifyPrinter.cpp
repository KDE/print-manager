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

#include "ModifyPrinter.h"

#include "ui_ModifyPrinter.h"

#include "Debug.h"
#include "SelectMakeModel.h"
#include "SelectMakeModelDialog.h"

#include <QPointer>
#include <QPushButton>

#include <KIconLoader>
#include <KPixmapSequenceOverlayPainter>
#include <KMessageBox>

ModifyPrinter::ModifyPrinter(const QString &destName, bool isClass, QWidget *parent) :
    PrinterPage(parent),
    ui(new Ui::ModifyPrinter),
    m_destName(destName),
    m_isClass(isClass),
    m_changes(0)
{
    ui->setupUi(this);

    ui->nameL->setText(destName);
    ui->connectionL->setVisible(!isClass);
    ui->connectionLE->setVisible(!isClass);
    ui->driverL->setVisible(!isClass);
    ui->makeCB->setVisible(!isClass);

    ui->membersL->setVisible(isClass);
    ui->membersLV->setVisible(isClass);
    if (isClass) {
        ui->membersLV->setPrinter(destName);
    }

    connect(ui->descriptionLE, &QLineEdit::textChanged, this, &ModifyPrinter::textChanged);
    connect(ui->locationLE, &QLineEdit::textChanged, this, &ModifyPrinter::textChanged);
    connect(ui->connectionLE, &QLineEdit::textChanged, this, &ModifyPrinter::textChanged);
    connect(ui->membersLV, static_cast<void(ClassListWidget::*)(bool)>(&ClassListWidget::changed),
            this, &ModifyPrinter::modelChanged);
}

ModifyPrinter::~ModifyPrinter()
{
    delete ui;
}

void ModifyPrinter::on_makeCB_activated(int index)
{
    bool isDifferent = true;
    if (ui->makeCB->itemData(index).toUInt() == PPDList) {
        auto dialog = new SelectMakeModelDialog(m_make, m_makeAndModel, this);
        connect(dialog, &SelectMakeModelDialog::accepted, this, &ModifyPrinter::ppdSelectionAccepted);
        connect(dialog, &SelectMakeModelDialog::rejected, this, &ModifyPrinter::ppdSelectionRejected);
        dialog->show();
        return;
    } else if (ui->makeCB->itemData(index).toUInt() == PPDFile) {
        // set the QVariant type to bool makes it possible to know a file was selected
        m_changedValues[QLatin1String("ppd-name")] = true;
    } else if (ui->makeCB->itemData(index).toUInt() == PPDDefault) {
        isDifferent = false;
        m_changedValues.remove(QLatin1String("ppd-name"));
    } else if (ui->makeCB->itemData(index).toUInt() == PPDCustom) {
        m_changedValues[QLatin1String("ppd-name")] = ui->makeCB->itemData(index, PPDName).toString();
    } else {
        qCWarning(PM_CONFIGURE_PRINTER) << "This should not happen";
        return;
    }

    if (isDifferent != ui->makeCB->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        ui->makeCB->setProperty("different", isDifferent);
        emit changed(m_changes);
    }
    ui->makeCB->setProperty("lastIndex", ui->makeCB->currentIndex());
}

void ModifyPrinter::ppdSelectionAccepted()
{
    auto dialog = qobject_cast<SelectMakeModelDialog*>(sender());
    auto widget = qobject_cast<SelectMakeModel*>(dialog->mainWidget());

    if (widget->isFileSelected()) {
        QString fileName = widget->selectedPPDFileName();
        ui->makeCB->insertItem(0, fileName, PPDFile);
        ui->makeCB->setCurrentIndex(0);
        on_makeCB_activated(0);
    } else if (!widget->selectedPPDName().isEmpty()) {
        QString makeAndModel = widget->selectedPPDMakeAndModel();
        QString ppdName = widget->selectedPPDName();
        ui->makeCB->insertItem(0, makeAndModel, PPDCustom);
        ui->makeCB->setItemData(0, ppdName, PPDName);
        ui->makeCB->setCurrentIndex(0);
        on_makeCB_activated(0);
    } else {
        ui->makeCB->setCurrentIndex(ui->makeCB->property("lastIndex").toInt());
    }

    dialog->deleteLater();
}

void ModifyPrinter::ppdSelectionRejected()
{
    auto dialog = qobject_cast<SelectMakeModelDialog*>(sender());
    ui->makeCB->setCurrentIndex(ui->makeCB->property("lastIndex").toInt());
    dialog->deleteLater();
}

void ModifyPrinter::setValues(const KCupsPrinter &printer)
{
//     qCDebug(PM_CONFIGURE_PRINTER) << values;
    if (m_isClass) {
        ui->membersLV->setSelectedPrinters(printer.memberNames().join(QLatin1String("|")));
    } else {
        ui->makeCB->clear();
        ui->makeCB->setProperty("different", false);
        ui->makeCB->setProperty("lastIndex", 0);
        ui->makeCB->insertItem(0,
                               i18n("Current - %1", printer.makeAndModel()),
                               PPDDefault);
        ui->makeCB->insertSeparator(1);
        ui->makeCB->insertItem(2, i18n("Select a custom driver"), PPDList);
    }
    ui->membersLV->setProperty("different", false);

    ui->descriptionLE->setText(printer.info());
    ui->descriptionLE->setProperty("orig_text", printer.info());
    ui->descriptionLE->setProperty("different", false);

    ui->locationLE->setText(printer.location());
    ui->locationLE->setProperty("orig_text", printer.location());
    ui->locationLE->setProperty("different", false);

    ui->connectionLE->setText(printer.deviceUri());
    ui->connectionLE->setProperty("orig_text", printer.deviceUri());
    ui->connectionLE->setProperty("different", false);

    // clear old values
    m_changes = 0;
    m_changedValues.clear();

    emit changed(0);
}

void ModifyPrinter::modelChanged()
{
    bool isDifferent = ui->membersLV->hasChanges();
    if (isDifferent != ui->membersLV->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        ui->membersLV->setProperty("different", isDifferent);
        emit changed(m_changes);
    }

    // store the new values
    if (isDifferent) {
        m_changedValues[KCUPS_MEMBER_URIS] = ui->membersLV->currentSelected(true);
    } else {
        m_changedValues.remove(KCUPS_MEMBER_URIS);
    }
}

void ModifyPrinter::textChanged(const QString &text)
{
    auto le = qobject_cast<QLineEdit *>(sender());

    bool isDifferent = le->property("orig_text") != text;
    if (isDifferent != le->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        le->setProperty("different", isDifferent);
        emit changed(m_changes);
    }

    // store the new values
    QString attribute = le->property("AttributeName").toString();
    if (isDifferent) {
        m_changedValues[attribute] = text;
    } else {
        m_changedValues.remove(attribute);
    }
}

void ModifyPrinter::save()
{
    if (m_changes) {
        QVariantHash args = m_changedValues;
        QString fileName;
        qCDebug(PM_CONFIGURE_PRINTER) << args;
        if (args.contains(QLatin1String("ppd-name")) &&
            args[QLatin1String("ppd-name")].type() == QVariant::Bool) {

            fileName = ui->makeCB->itemData(ui->makeCB->currentIndex(), PPDFile).toString();
            args.remove(QLatin1String("ppd-name"));
        }
        qCDebug(PM_CONFIGURE_PRINTER) << fileName;

        QPointer<KCupsRequest> request = new KCupsRequest;
        if (m_isClass) {
            request->addOrModifyClass(m_destName, args);
        } else {
            request->addOrModifyPrinter(m_destName, args, fileName);
        }
        request->waitTillFinished();
        if (request) {
            if (!request->hasError()) {
                if (m_changedValues.contains(QLatin1String("ppd-name"))) {
                    emit ppdChanged();
                }
                request->getPrinterAttributes(m_destName, m_isClass, neededValues());
                request->waitTillFinished();

                if (!request->hasError() && !request->printers().isEmpty()) {
                    KCupsPrinter printer = request->printers().first();
                    setValues(printer);
                }
            } else {
                KMessageBox::detailedSorry(this,
                                           m_isClass ? i18nc("@info", "Failed to configure class") :
                                                       i18nc("@info", "Failed to configure printer"),
                                           request->errorMsg(),
                                           i18nc("@title:window", "Failed"));
            }
            request->deleteLater();
        }
    }
}

QVariantHash ModifyPrinter::modifiedValues() const
{
    return m_changedValues;
}

bool ModifyPrinter::hasChanges()
{
    return m_changes;
}

void ModifyPrinter::setRemote(bool remote)
{
    ui->descriptionLE->setReadOnly(remote);
    ui->locationLE->setReadOnly(remote);
    ui->connectionLE->setReadOnly(remote);
    ui->makeCB->setEnabled(!remote);
}

void ModifyPrinter::setCurrentMake(const QString &make)
{
    m_make = make;
}

void ModifyPrinter::setCurrentMakeAndModel(const QString &makeAndModel)
{
    m_makeAndModel = makeAndModel;
}

QStringList ModifyPrinter::neededValues() const
{
    QStringList ret;
    ret << KCUPS_PRINTER_INFO;
    ret << KCUPS_PRINTER_LOCATION;

    if (m_isClass) {
        ret << KCUPS_MEMBER_NAMES;
    } else {
        ret << KCUPS_DEVICE_URI;
        ret << KCUPS_PRINTER_MAKE_AND_MODEL;
    }
    return ret;
}
