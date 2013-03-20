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

#include "PrinterBehavior.h"
#include "ui_PrinterBehavior.h"

#include <KComboBox>
#include <KDebug>

PrinterBehavior::PrinterBehavior(const QString &destName, bool isClass, QWidget *parent) :
    PrinterPage(parent),
    ui(new Ui::PrinterBehavior),
    m_destName(destName),
    m_isClass(isClass),
    m_changes(0)
{
    ui->setupUi(this);

    connect(ui->errorPolicyCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));
    connect(ui->operationPolicyCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));

    connect(ui->startingBannerCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));
    connect(ui->endingBannerCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));

    connect(ui->usersELB, SIGNAL(changed()),
            this, SLOT(userListChanged()));
    connect(ui->allowRB, SIGNAL(toggled(bool)),
            this, SLOT(userListChanged()));
}

PrinterBehavior::~PrinterBehavior()
{
    delete ui;
}

void PrinterBehavior::setValues(const KCupsPrinter &printer)
{
    int defaultChoice;
    ui->errorPolicyCB->clear();
    foreach (const QString &value, printer.errorPolicySupported()) {
        ui->errorPolicyCB->addItem(errorPolicyString(value), value);
    }
    QStringList errorPolicy = printer.errorPolicy();
    if (!errorPolicy.isEmpty()) {
        defaultChoice = ui->errorPolicyCB->findData(errorPolicy.first());
        ui->errorPolicyCB->setCurrentIndex(defaultChoice);
        ui->errorPolicyCB->setProperty("defaultChoice", defaultChoice);
    }

    ui->operationPolicyCB->clear();
    foreach (const QString &value, printer.opPolicySupported()) {
        ui->operationPolicyCB->addItem(operationPolicyString(value), value);
    }
    QStringList operationPolicy = printer.opPolicy();
    if (!errorPolicy.isEmpty()) {
        defaultChoice = ui->operationPolicyCB->findData(operationPolicy.first());
        ui->operationPolicyCB->setCurrentIndex(defaultChoice);
        ui->operationPolicyCB->setProperty("defaultChoice", defaultChoice);
    }

    ui->startingBannerCB->clear();
    ui->endingBannerCB->clear();
    foreach (const QString &value, printer.jobSheetsSupported()) {
        ui->startingBannerCB->addItem(jobSheetsString(value), value);
        ui->endingBannerCB->addItem(jobSheetsString(value), value);
    }
    QStringList bannerPolicy = printer.jobSheetsDefault();
    if (bannerPolicy.size() == 2) {
        defaultChoice = ui->startingBannerCB->findData(bannerPolicy.at(0));
        ui->startingBannerCB->setCurrentIndex(defaultChoice);
        ui->startingBannerCB->setProperty("defaultChoice", defaultChoice);
        defaultChoice = ui->endingBannerCB->findData(bannerPolicy.at(1));
        ui->endingBannerCB->setCurrentIndex(defaultChoice);
        ui->endingBannerCB->setProperty("defaultChoice", defaultChoice);
    }

    if (!printer.requestingUserNameAllowed().isEmpty()) {
        QStringList list = printer.requestingUserNameAllowed();
        list.sort(); // sort the list here to be able to comapare it later
        ui->usersELB->setEnabled(true);
        if (list != ui->usersELB->items()) {
            ui->usersELB->clear();
            ui->usersELB->insertStringList(list);
        }
        ui->usersELB->setProperty("defaultList", list);
        ui->allowRB->setProperty("defaultChoice", true);
        // Set checked AFTER the default choice was set
        // otherwise the signal will be emmited
        // which sets that we have a change
        ui->allowRB->setChecked(true);

    } else if (!printer.requestingUserNameDenied().isEmpty()) {
        QStringList list = printer.requestingUserNameDenied();
        list.sort(); // sort the list here to be able to comapare it later
        ui->usersELB->setEnabled(true);
        if (list != ui->usersELB->items()) {
            ui->usersELB->clear();
            ui->usersELB->insertStringList(list);
        }
        ui->usersELB->setProperty("defaultList", list);
        ui->allowRB->setProperty("defaultChoice", false);
        // Set checked AFTER the default choice was set
        // otherwise the signal will be emmited
        // which sets that we have a change
        ui->preventRB->setChecked(true);
    }

    // Clear previous changes
    m_changes = 0;
    emit changed(false);
    m_changedValues.clear();
    ui->errorPolicyCB->setProperty("different", false);
    ui->operationPolicyCB->setProperty("different", false);
    ui->startingBannerCB->setProperty("different", false);
    ui->endingBannerCB->setProperty("different", false);
    ui->usersELB->setProperty("different", false);
}

void PrinterBehavior::userListChanged()
{
    if (ui->usersELB->isEnabled() == false &&
        (ui->allowRB->isChecked() ||
         ui->preventRB->isChecked())) {
        // this only happen when the list was empty
       ui-> usersELB->setEnabled(true);
    }

    QStringList currentList, defaultList;
    currentList = ui->usersELB->items();
    // sort the list so we can be sure it's different
    currentList.sort();
    defaultList = ui->usersELB->property("defaultList").value<QStringList>();

    bool isDifferent = currentList != defaultList;
    if (isDifferent == false && currentList.isEmpty() == false) {
        // if the lists are equal and not empty the user might have
        // changed the Radio Button...
        if (ui->allowRB->isChecked() != ui->allowRB->property("defaultChoice").toBool()) {
            isDifferent = true;
        }
    }

    if (isDifferent != ui->usersELB->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        ui->usersELB->setProperty("different", isDifferent);
        emit changed(m_changes);
    }
}

void PrinterBehavior::currentIndexChangedCB(int index)
{
    KComboBox *comboBox = qobject_cast<KComboBox*>(sender());
    bool isDifferent = comboBox->property("defaultChoice").toInt() != index;

    if (isDifferent != comboBox->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        comboBox->setProperty("different", isDifferent);
        emit changed(m_changes);
    }

    QString attribute = comboBox->property("AttributeName").toString();
    QVariant value;
    // job-sheets-default has always two values
    if (attribute == "job-sheets-default") {
        QStringList values;
        values << ui->startingBannerCB->itemData(ui->startingBannerCB->currentIndex()).toString();
        values << ui->endingBannerCB->itemData(ui->endingBannerCB->currentIndex()).toString();
        value = values;
    } else {
        value = comboBox->itemData(index).toString();
    }

    // store the new values
    if (isDifferent) {
        m_changedValues[attribute] = value;
    } else {
        m_changedValues.remove(attribute);
    }
}

QString PrinterBehavior::errorPolicyString(const QString &policy) const
{
    // TODO search for others policies of printer-error-policy-supported
    if (policy == "abort-job") {
        return i18n("Abort job");
    } else if (policy == "retry-current-job") {
        return i18n("Retry current job");
    } else if (policy == "retry-job") {
        return i18n("Retry job");
    } else if (policy == "stop-printer") {
        return i18n("Stop printer");
    }
    return policy;
}

QString PrinterBehavior::operationPolicyString(const QString &policy) const
{
    // TODO search for others policies of printer-error-policy-supported
    if (policy == "authenticated") {
        return i18n("Authenticated");
    } else if (policy == "default") {
        return i18n("Default");
    }
    return policy;
}

QString PrinterBehavior::jobSheetsString(const QString &policy) const
{
    // TODO search for others policies of printer-error-policy-supported
    if (policy == "none") {
        return i18n("None");
    } else if (policy == "classified") {
        return i18n("Classified");
    } else if (policy == "confidential") {
        return i18n("Confidential");
    } else if (policy == "secret") {
        return i18n("Secret");
    } else if (policy == "standard") {
        return i18n("Standard");
    } else if (policy == "topsecret") {
        return i18n("Topsecret");
    } else if (policy == "unclassified") {
        return i18n("Unclassified");
    }
    return policy;
}

void PrinterBehavior::save()
{
    if (m_changes) {
        QVariantHash changedValues = m_changedValues;
        // since a QStringList might be big we get it here instead
        // of adding it at edit time.
        if (ui->usersELB->property("different").toBool()) {
            QStringList list = ui->usersELB->items();
            if (list.isEmpty()) {
                list << "all";
                changedValues["requesting-user-name-allowed"] = list;
            } else {
                if (ui->allowRB->isChecked()) {
                    changedValues["requesting-user-name-allowed"] = list;
                } else {
                    changedValues["requesting-user-name-denied"] = list;
                }
            }
        }
        QPointer<KCupsRequest> request = new KCupsRequest;
        if (m_isClass) {
            request->addOrModifyClass(m_destName, changedValues);
        } else {
            request->addOrModifyPrinter(m_destName, changedValues);
        }
        request->waitTillFinished();
        if (request) {
            if (!request->hasError()) {
                request->getPrinterAttributes(m_destName, m_isClass, neededValues());
                request->waitTillFinished();
                if (request && !request->hasError() && !request->printers().isEmpty()){
                    KCupsPrinter printer = request->printers().first();
                    setValues(printer);
                }
            }
            request->deleteLater();
        }
    }
}

void PrinterBehavior::setRemote(bool remote)
{
    ui->errorPolicyCB->setEnabled(!remote);
    ui->operationPolicyCB->setEnabled(!remote);
    ui->startingBannerCB->setEnabled(!remote);
    ui->endingBannerCB->setEnabled(!remote);
    ui->allowRB->setEnabled(!remote);
    ui->preventRB->setEnabled(!remote);
    ui->usersELB->setEnabled(!remote);
}

bool PrinterBehavior::hasChanges()
{
    return m_changes;
}

QStringList PrinterBehavior::neededValues() const
{
    QStringList ret;
    ret << KCUPS_JOB_SHEETS_DEFAULT;
    ret << KCUPS_JOB_SHEETS_SUPPORTED;

    ret << KCUPS_PRINTER_ERROR_POLICY;
    ret << KCUPS_PRINTER_ERROR_POLICY_SUPPORTED;

    ret << KCUPS_PRINTER_OP_POLICY;
    ret << KCUPS_PRINTER_OP_POLICY_SUPPORTED;

    ret << KCUPS_REQUESTING_USER_NAME_ALLOWED;
    ret << KCUPS_REQUESTING_USER_NAME_DENIED;

    return ret;
}
