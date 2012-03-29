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

#include "ModifyPrinter.h"

#include "ui_ModifyPrinter.h"

#include "SelectMakeModel.h"

#include <QPointer>
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
    ui->makeModelCB->setVisible(!isClass);

    ui->membersL->setVisible(isClass);
    ui->membersLV->setVisible(isClass);

    connect(ui->descriptionLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(ui->locationLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(ui->connectionLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(ui->membersLV, SIGNAL(changed(bool)),
            this, SLOT(modelChanged()));

    connect(this, SIGNAL(showKUR()), ui->fileKUR, SLOT(show()));
    connect(this, SIGNAL(showKUR()), ui->fileL, SLOT(show()));
    connect(this, SIGNAL(hideKUR()), ui->fileKUR, SLOT(clear()));
    connect(this, SIGNAL(hideKUR()), ui->fileKUR, SLOT(hide()));
    connect(this, SIGNAL(hideKUR()), ui->fileL, SLOT(hide()));
}

ModifyPrinter::~ModifyPrinter()
{
    delete ui;
}

void ModifyPrinter::on_makeCB_activated(int index)
{
    bool isDifferent = true;
    if (ui->makeCB->itemData(index).toUInt() == PPDList) {
        emit hideKUR();
        KConfig config("print-manager");
        KConfigGroup ppdDialog(&config, "PPDDialog");

        SelectMakeModel *widget = new SelectMakeModel(this);
        widget->setMakeModel(m_make, m_makeAndModel);

        QPointer<KDialog> dialog = new KDialog(this);
        dialog->setCaption("Select a Driver");
        dialog->setButtons(KDialog::Ok | KDialog::Cancel);
        dialog->setMainWidget(widget);
        connect(widget, SIGNAL(changed(bool)),
                dialog, SLOT(enableButtonOk(bool)));
        dialog->enableButtonOk(false);
        dialog->restoreDialogSize(ppdDialog);
        if (dialog->exec() == QDialog::Accepted && dialog) {
            dialog->saveDialogSize(ppdDialog);
            QString makeAndModel = widget->selectedMakeAndModel();
            QString ppdName = widget->selectedPPDName();
            if (!ppdName.isEmpty() && !makeAndModel.isEmpty()){
                ui->makeCB->insertItem(0, makeAndModel, PPDCustom);
                ui->makeCB->setItemData(0, ppdName, PPDName);
                ui->makeCB->setCurrentIndex(0);
                // store the new value
                m_changedValues["ppd-name"] = ppdName;
            } else {
                ui->makeCB->setCurrentIndex(ui->makeCB->property("lastIndex").toInt());
            }
        } else {
            ui->makeCB->setCurrentIndex(ui->makeCB->property("lastIndex").toInt());
        }
        return;
    } else if (ui->makeCB->itemData(index).toUInt() == PPDFile) {
        ui->fileKUR->button()->click();
        if (ui->fileKUR->url().isEmpty()) {
            ui->makeCB->setCurrentIndex(ui->makeCB->property("lastIndex").toInt());
            return;
        }
        emit showKUR();
        // set the QVariant type to bool makes it possible to know a file was selected
        m_changedValues["ppd-name"] = true;
    } else if (ui->makeCB->itemData(index).toUInt() == PPDDefault) {
        isDifferent = false;
        m_changedValues.remove("ppd-name");
        emit hideKUR();
    } else if (ui->makeCB->itemData(index).toUInt() == PPDCustom) {
        emit hideKUR();
        m_changedValues["ppd-name"] = ui->makeCB->itemData(index, PPDName).toString();
    } else {
        emit hideKUR();
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

void ModifyPrinter::setValues(const QVariantHash &values)
{
//     kDebug() << values;
    if (m_isClass) {
        ui->membersLV->reload(m_destName, values["member-names"].toStringList());
    } else {
        emit hideKUR();
        ui->makeCB->clear();
        ui->makeCB->setProperty("different", false);
        ui->makeCB->setProperty("lastIndex", 0);
        ui->makeCB->insertItem(0,
                               i18n("Current - %1", values["printer-make-and-model"].toString()),
                               PPDDefault);
        ui->makeCB->insertSeparator(1);
        ui->makeCB->insertItem(2, i18n("Select a Driver from a List"), PPDList);
        ui->makeCB->insertItem(3, i18n("Provide a PPD file"), PPDFile);
    }

    ui->descriptionLE->setText(values["printer-info"].toString());
    ui->descriptionLE->setProperty("orig_text", values["printer-info"].toString());

    ui->locationLE->setText(values["printer-location"].toString());
    ui->locationLE->setProperty("orig_text", values["printer-location"].toString());

    ui->connectionLE->setText(values["device-uri"].toString());
    ui->connectionLE->setProperty("orig_text", values["device-uri"].toString());

    // clear old values
    m_changes = 0;
    m_changedValues.clear();
    ui->descriptionLE->setProperty("different", false);
    ui->locationLE->setProperty("different", false);
    ui->connectionLE->setProperty("different", false);
    ui->membersLV->setProperty("different", false);
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
        QString file;
        if (m_changedValues.contains("ppd-name") &&
            m_changedValues["ppd-name"].type() == QVariant::Bool) {
            // check if it's really a local file and set the file string
            if (ui->fileKUR->url().isLocalFile()) {
                file = ui->fileKUR->url().toLocalFile();
            }
            m_changedValues.remove("ppd-name");
        }
        // if there is no file call setAttributes witout it
        KCupsRequest *request = new KCupsRequest;
        if (file.isEmpty()) {
            kDebug() << m_changedValues;
            request->setAttributes(m_destName, m_isClass, m_changedValues);
        } else {
            request->setAttributes(m_destName, m_isClass, m_changedValues, file.toUtf8());
        }
        request->waitTillFinished();

        if (!request->hasError()) {
            if (!file.isEmpty() ||
                (m_changedValues.contains("ppd-name") && m_changedValues["ppd-name"].type() != QVariant::Bool)) {
                emit ppdChanged();
            }
            request->getAttributes(m_destName, m_isClass, neededValues());
            request->waitTillFinished();

            if (!request->hasError() && !request->result().isEmpty()) {
                QVariantHash attributes = request->result().first();
                setValues(attributes);
            }
        }
        request->deleteLater();
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
    QStringList values;
    values << "printer-info"
           << "printer-location";
    if (m_isClass) {
        values << "member-names";
    } else {
        values << "device-uri"
               << "printer-make-and-model";
    }
    return values;
}

#include "ModifyPrinter.moc"
