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

#include "SelectMakeModel.h"

#include "QCups.h"
#include <cups/cups.h>

#include <QPointer>
#include <KFileDialog>
#include <KDebug>

using namespace QCups;

ModifyPrinter::ModifyPrinter(const QString &destName, bool isClass, bool isModify, QWidget *parent)
 : PrinterPage(parent), m_destName(destName), m_isClass(isClass), m_changes(0)
{
    setupUi(this);

    if (isModify) {
        // we are modifying the printer/class so
        // the user cannot change it.
        nameLE->setText(destName);
        nameLE->setReadOnly(true);
    }

    connectionL->setVisible(!isClass);
    connectionLE->setVisible(!isClass);
    makeModelCB->setVisible(!isClass);

    membersL->setVisible(isClass);
    membersLV->setVisible(isClass);

    m_model = new QStandardItemModel(membersLV);
    membersLV->setModel(m_model);

    connect(descriptionLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(locationLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(connectionLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(modelChanged()));

    connect(this, SIGNAL(showKUR()), fileKUR, SLOT(show()));
    connect(this, SIGNAL(showKUR()), fileL, SLOT(show()));
    connect(this, SIGNAL(hideKUR()), fileKUR, SLOT(clear()));
    connect(this, SIGNAL(hideKUR()), fileKUR, SLOT(hide()));
    connect(this, SIGNAL(hideKUR()), fileL, SLOT(hide()));
}

ModifyPrinter::~ModifyPrinter()
{
}

void ModifyPrinter::on_makeCB_activated(int index)
{
    bool isDifferent = true;
    if (makeCB->itemData(index).toUInt() == PPDList) {
        emit hideKUR();
        KConfig config("print-manager");
        KConfigGroup ppdDialog(&config, "PPDDialog");
        QPointer<KDialog> dialog = new KDialog(this);
        dialog->setCaption("Select a Driver");
        dialog->setButtons(KDialog::Ok | KDialog::Cancel);
        SelectMakeModel *widget = new SelectMakeModel(m_make, m_makeAndModel, this);
        dialog->setMainWidget(widget);
        connect(widget, SIGNAL(changed(bool)),
                dialog, SLOT(enableButtonOk(bool)));
        // Call this to disable the Ok button
        widget->checkChanged();

        dialog->restoreDialogSize(ppdDialog);
        if (dialog->exec() == QDialog::Accepted && dialog) {
            dialog->saveDialogSize(ppdDialog);
            QString makeAndModel = widget->selectedMakeAndModel();
            QString ppdName = widget->selectedPPDName();
            if (!ppdName.isEmpty() && !makeAndModel.isEmpty()){
                makeCB->insertItem(0, makeAndModel, PPDCustom);
                makeCB->setItemData(0, ppdName, PPDName);
                makeCB->setCurrentIndex(0);
                // store the new value
                m_changedValues["ppd-name"] = ppdName;
            } else {
                makeCB->setCurrentIndex(makeCB->property("lastIndex").toInt());
                return;
            }
        } else {
            makeCB->setCurrentIndex(makeCB->property("lastIndex").toInt());
            return;
        }
    } else if (makeCB->itemData(index).toUInt() == PPDFile) {
        fileKUR->button()->click();
        if (fileKUR->url().isEmpty()) {
            makeCB->setCurrentIndex(makeCB->property("lastIndex").toInt());
            return;
        }
        emit showKUR();
        // set the QVariant type to bool makes it possible to know a file was selected
        m_changedValues["ppd-name"] = true;
    } else if (makeCB->itemData(index).toUInt() == PPDDefault) {
        isDifferent = false;
        m_changedValues.remove("ppd-name");
        emit hideKUR();
    } else if (makeCB->itemData(index).toUInt() == PPDCustom) {
        emit hideKUR();
        m_changedValues["ppd-name"] = makeCB->itemData(index, PPDName).toString();
    } else {
        emit hideKUR();
        kWarning() << "This should not happen";
        return;
    }
//     kDebug() << isDifferent << makeCB->property("different").toBool();

    if (isDifferent != makeCB->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        makeCB->setProperty("different", isDifferent);
        emit changed(m_changes);
    }
    makeCB->setProperty("lastIndex", makeCB->currentIndex());
}

void ModifyPrinter::setValues(const QHash<QString, QVariant> &values)
{
//     kDebug() << values;
    if (m_isClass) {
        ReturnArguments dests;
        // Ask just these attributes
        QStringList requestAttr;
        requestAttr << "printer-uri-supported"
                    << "printer-name";
        // Get destinations with these masks
        Result *ret = QCups::getDests(CUPS_PRINTER_CLASS | CUPS_PRINTER_REMOTE |
                                      CUPS_PRINTER_IMPLICIT, requestAttr);

        dests = ret->result();
        QEventLoop loop;
        connect(ret, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        ret->deleteLater();

        m_model->clear();
        QStringList memberNames = values["member-names"].toStringList();
        QStringList origMemberUris;
        foreach (const QString &memberUri, memberNames) {
            for (int i = 0; i < dests.size(); i++) {
                if (dests.at(i)["printer-name"].toString() == memberUri) {
                    origMemberUris << dests.at(i)["printer-uri-supported"].toString();
                    break;
                }
            }
        }
        m_model->setProperty("orig-member-uris", origMemberUris);

        for (int i = 0; i < dests.size(); i++) {
            QString destName = dests.at(i)["printer-name"].toString();
            if (destName != m_destName) {
                QStandardItem *item = new QStandardItem(destName);
                item->setCheckable(true);
                item->setEditable(false);
                if (memberNames.contains(destName)) {
                    item->setCheckState(Qt::Checked);
                }
                item->setData(dests.at(i)["printer-uri-supported"].toString());
                m_model->appendRow(item);
            }
        }
    } else {
        emit hideKUR();
        makeCB->clear();
        makeCB->setProperty("different", false);
        makeCB->setProperty("lastIndex", 0);
        makeCB->insertItem(0,
                           i18n("Current - %1", values["printer-make-and-model"].toString()),
                           PPDDefault);
        makeCB->insertSeparator(1);
        makeCB->insertItem(2, i18n("Select a Driver from a List"), PPDList);
        makeCB->insertItem(3, i18n("Provide a PPD file"), PPDFile);
    }

    descriptionLE->setText(values["printer-info"].toString());
    descriptionLE->setProperty("orig_text", values["printer-info"].toString());

    locationLE->setText(values["printer-location"].toString());
    locationLE->setProperty("orig_text", values["printer-location"].toString());

    connectionLE->setText(values["device-uri"].toString());
    connectionLE->setProperty("orig_text", values["device-uri"].toString());

    // clear old values
    m_changes = 0;
    m_changedValues.clear();
    descriptionLE->setProperty("different", false);
    locationLE->setProperty("different", false);
    connectionLE->setProperty("different", false);
    m_model->setProperty("different", false);
    emit changed(0);
}

void ModifyPrinter::modelChanged()
{
    QStringList currentMembers;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QStandardItem *item = m_model->item(i);
        if (item && item->checkState() == Qt::Checked) {
            currentMembers << item->data().toString();
        }
    }
    currentMembers.sort();

    bool isDifferent = m_model->property("orig-member-uris").toStringList() != currentMembers;
    if (isDifferent != m_model->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        m_model->setProperty("different", isDifferent);
        emit changed(m_changes);
    }

    // store the new values
    if (isDifferent) {
        m_changedValues["member-uris"] = currentMembers;
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
            if (fileKUR->url().isLocalFile()) {
                file = fileKUR->url().toLocalFile();
            }
            m_changedValues.remove("ppd-name");
        }
        // if there is no file call setAttributes witout it
        Result *result;
        if (file.isEmpty()) {
            result = Dest::setAttributes(m_destName, m_isClass, m_changedValues);
        } else {
            result = Dest::setAttributes(m_destName, m_isClass, m_changedValues, file.toUtf8());
        }

        if (result && !result->lastError()) {
            if (!file.isEmpty() ||
                (m_changedValues.contains("ppd-name") && m_changedValues["ppd-name"].type() != QVariant::Bool)) {
                emit ppdChanged();
            }
            Result *ret = Dest::getAttributes(m_destName, m_isClass, neededValues());
            if (!ret->result().isEmpty()){
                QHash<QString, QVariant> attributes = ret->result().first();
                setValues(attributes);
            }
            delete ret;
        }
        delete result;
    }
}

QHash<QString, QVariant> ModifyPrinter::modifiedValues() const
{
    return m_changedValues;
}

bool ModifyPrinter::hasChanges()
{
    return m_changes;
}

void ModifyPrinter::setRemote(bool remote)
{
    descriptionLE->setReadOnly(remote);
    locationLE->setReadOnly(remote);
    connectionLE->setReadOnly(remote);
    makeCB->setEnabled(!remote);
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
