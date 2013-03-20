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

#include "ModifyPrinter.h"

#include "ui_ModifyPrinter.h"

#include "SelectMakeModel.h"

#include <KDialog>
#include <KPixmapSequenceOverlayPainter>
#include <KPixmapSequence>
#include <KPushButton>
#include <KMessageBox>

#include <KDebug>

ModifyPrinter::ModifyPrinter(const QString &destName, bool isClass, bool isModify, QWidget *parent) :
    PrinterPage(parent),
    ui(new Ui::ModifyPrinter),
    m_destName(destName),
    m_isClass(isClass),
    m_changes(0)
{
    ui->setupUi(this);

    if (isModify) {
        // we are modifying the printer/class so
        // the user cannot change it.
        ui->nameLE->setText(destName);
        ui->nameLE->setReadOnly(true);
    }

    ui->connectionL->setVisible(!isClass);
    ui->connectionLE->setVisible(!isClass);
    ui->driverL->setVisible(!isClass);
    ui->makeCB->setVisible(!isClass);

    ui->membersL->setVisible(isClass);
    ui->membersLV->setVisible(isClass);

    connect(ui->descriptionLE, SIGNAL(textChanged(QString)),
            this, SLOT(textChanged(QString)));
    connect(ui->locationLE, SIGNAL(textChanged(QString)),
            this, SLOT(textChanged(QString)));
    connect(ui->connectionLE, SIGNAL(textChanged(QString)),
            this, SLOT(textChanged(QString)));
    connect(ui->membersLV, SIGNAL(changed(bool)),
            this, SLOT(modelChanged()));
}

ModifyPrinter::~ModifyPrinter()
{
    delete ui;
}

void ModifyPrinter::on_makeCB_activated(int index)
{
    bool isDifferent = true;
    if (ui->makeCB->itemData(index).toUInt() == PPDList) {
        KConfig config("print-manager");
        KConfigGroup ppdDialog(&config, "PPDDialog");

        SelectMakeModel *widget = new SelectMakeModel(this);

        KDialog *dialog = new KDialog(this);
        connect(dialog, SIGNAL(accepted()), this, SLOT(ppdSelectionAccepted()));
        connect(dialog, SIGNAL(rejected()), this, SLOT(ppdSelectionRejected()));
        connect(widget, SIGNAL(changed(bool)), dialog, SLOT(enableButtonOk(bool)));
        dialog->setCaption("Select a Driver");
        dialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);
        dialog->setMainWidget(widget);
        dialog->enableButtonOk(false);
        dialog->restoreDialogSize(ppdDialog);

        // Configure the help button to be flat, disabled and empty
        KPushButton *button = dialog->button(KDialog::Help);
        button->setFlat(true);
        button->setEnabled(false);
        button->setIcon(QIcon());
        button->setText(QString());

        // Setup the busy cursor
        KPixmapSequenceOverlayPainter *busySeq = new KPixmapSequenceOverlayPainter(dialog);
        busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
        busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        busySeq->setWidget(button);
        busySeq->start();
        connect(widget, SIGNAL(changed(bool)), busySeq, SLOT(stop()));
kDebug() << m_make << m_makeAndModel;
        widget->setMakeModel(m_make, m_makeAndModel);
        dialog->show();
        return;
    } else if (ui->makeCB->itemData(index).toUInt() == PPDFile) {
        // set the QVariant type to bool makes it possible to know a file was selected
        m_changedValues["ppd-name"] = true;
    } else if (ui->makeCB->itemData(index).toUInt() == PPDDefault) {
        isDifferent = false;
        m_changedValues.remove("ppd-name");
    } else if (ui->makeCB->itemData(index).toUInt() == PPDCustom) {
        m_changedValues["ppd-name"] = ui->makeCB->itemData(index, PPDName).toString();
    } else {
        kWarning() << "This should not happen";
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
    KDialog *dialog = qobject_cast<KDialog*>(sender());
    SelectMakeModel *widget = qobject_cast<SelectMakeModel*>(dialog->mainWidget());

    if (widget->isFileSelected()) {
        QString fileName = widget->selectedPPDFileName();
        ui->makeCB->insertItem(0, fileName, PPDFile);
        ui->makeCB->setCurrentIndex(0);
        on_makeCB_activated(0);
    } else if (!widget->selectedPPDName().isNull()) {
        QString makeAndModel = widget->selectedPPDMakeAndModel();
        QString ppdName = widget->selectedPPDName();
        ui->makeCB->insertItem(0, makeAndModel, PPDCustom);
        ui->makeCB->setItemData(0, ppdName, PPDName);
        ui->makeCB->setCurrentIndex(0);
        on_makeCB_activated(0);
    } else {
        ui->makeCB->setCurrentIndex(ui->makeCB->property("lastIndex").toInt());
    }

    KConfig config("print-manager");
    KConfigGroup ppdDialog(&config, "PPDDialog");
    dialog->saveDialogSize(ppdDialog);
    dialog->deleteLater();
}

void ModifyPrinter::ppdSelectionRejected()
{
    ui->makeCB->setCurrentIndex(ui->makeCB->property("lastIndex").toInt());

    KDialog *dialog = qobject_cast<KDialog*>(sender());
    KConfig config("print-manager");
    KConfigGroup ppdDialog(&config, "PPDDialog");
    dialog->saveDialogSize(ppdDialog);
    dialog->deleteLater();
}

void ModifyPrinter::setValues(const KCupsPrinter &printer)
{
//     kDebug() << values;
    if (m_isClass) {
        ui->membersLV->reload(m_destName, printer.memberNames());
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
        m_changedValues["member-uris"] = ui->membersLV->selectedDests();
    } else {
        m_changedValues.remove("member-uris");
    }
}

void ModifyPrinter::textChanged(const QString &text)
{
    KLineEdit *le = qobject_cast<KLineEdit *>(sender());

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
        kDebug() << args;
        if (args.contains("ppd-name") &&
            args["ppd-name"].type() == QVariant::Bool) {

            fileName = ui->makeCB->itemData(ui->makeCB->currentIndex(), PPDFile).toString();
            args.remove("ppd-name");
        }
        kDebug() << fileName;

        QPointer<KCupsRequest> request = new KCupsRequest;
        if (m_isClass) {
            request->addOrModifyClass(m_destName, args);
        } else {
            request->addOrModifyPrinter(m_destName, args, fileName);
        }
        request->waitTillFinished();
        if (request) {
            if (!request->hasError()) {
                if (m_changedValues.contains("ppd-name")) {
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

#include "ModifyPrinter.moc"
