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

#include "PrinterBehavior.h"

#include <QCups.h>

#include <KDebug>

using namespace QCups;

PrinterBehavior::PrinterBehavior(const QString &destName, QWidget *parent)
 : PrinterPage(parent), m_destName(destName), m_changes(0)
{
    setupUi(this);

    connect(errorPolicyCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));
    connect(operationPolicyCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));

    connect(startingBannerCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));
    connect(endingBannerCB, SIGNAL(currentIndexChanged(int)),
            this, SLOT(currentIndexChangedCB(int)));
}

PrinterBehavior::~PrinterBehavior()
{
}

void PrinterBehavior::setValues(const QHash<QString, QVariant> &values)
{
    int defaultChoice;
    foreach (const QString &value, values["printer-error-policy-supported"].value<QStringList>()) {
        errorPolicyCB->addItem(errorPolicyString(value), value);
    }
    QStringList errorPolicy = values["printer-error-policy"].value<QStringList>();
    if (!errorPolicy.isEmpty()) {
        defaultChoice = errorPolicyCB->findData(errorPolicy.first());
        errorPolicyCB->setCurrentIndex(defaultChoice);
        errorPolicyCB->setProperty("defaultChoice", defaultChoice);
    }

    foreach (const QString &value, values["printer-op-policy-supported"].value<QStringList>()) {
        operationPolicyCB->addItem(operationPolicyString(value), value);
    }
    QStringList operationPolicy = values["printer-op-policy"].value<QStringList>();
    if (!errorPolicy.isEmpty()) {
        defaultChoice = operationPolicyCB->findData(operationPolicy.first());
        operationPolicyCB->setCurrentIndex(defaultChoice);
        operationPolicyCB->setProperty("defaultChoice", defaultChoice);
    }

    foreach (const QString &value, values["job-sheets-supported"].value<QStringList>()) {
        startingBannerCB->addItem(jobSheetsString(value), value);
        endingBannerCB->addItem(jobSheetsString(value), value);
    }
    QStringList bannerPolicy = values["job-sheets-default"].value<QStringList>();
    if (bannerPolicy.size() == 2) {
        defaultChoice = startingBannerCB->findData(bannerPolicy.at(0));
        startingBannerCB->setCurrentIndex(defaultChoice);
        startingBannerCB->setProperty("defaultChoice", defaultChoice);
        defaultChoice = endingBannerCB->findData(bannerPolicy.at(1));
        endingBannerCB->setCurrentIndex(defaultChoice);
        endingBannerCB->setProperty("defaultChoice", defaultChoice);
    }

    if (values.contains("requesting-user-name-allowed")) {
        allowRB->setChecked(true);
        usersELB->insertStringList(values["requesting-user-name-allowed"].value<QStringList>());
    } else if (values.contains("requesting-user-name-denied")) {
        preventRB->setChecked(true);
        usersELB->insertStringList(values["requesting-user-name-denied"].value<QStringList>());
    }

    // Clear previous changes
    m_changes = 0;
    m_changedValues.clear();
    errorPolicyCB->setProperty("different", false);
    operationPolicyCB->setProperty("different", false);
    startingBannerCB->setProperty("different", false);
    endingBannerCB->setProperty("different", false);
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
        values << startingBannerCB->itemData(startingBannerCB->currentIndex()).toString();
        values << endingBannerCB->itemData(endingBannerCB->currentIndex()).toString();
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
        kDebug() << m_changedValues;
        QCups::Printer::setAttributes(m_destName, m_changedValues);
//         QHash<QString, QVariant> values;
//         if (nameLE->property("different").toBool()) {
//             values["printer-info"] = nameLE->text();
//         }
//         if (locationLE->property("different").toBool()) {
//             values["printer-location"] = locationLE->text();
//         }
//         if (connectionLE->property("different").toBool()) {
//             values["device-uri"] = connectionLE->text();
//         }
//         m_printer->save(values);
    }
}

bool PrinterBehavior::hasChanges()
{
    return m_changes;
}

#include "PrinterBehavior.moc"
