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
 : PrinterPage(parent), m_changes(0)
{
    setupUi(this);

//     m_printer = new Printer(destName, this);

//     makeCB->addItem(m_printer->value("printer-make-and-model"));
//     nameLE->setText(m_printer->value("printer-info"));
//     nameLE->setProperty("orig_text", m_printer->value("printer-info"));
//     locationLE->setText(m_printer->value("printer-location"));
//     locationLE->setProperty("orig_text", m_printer->value("printer-location"));
//     connectionLE->setText(m_printer->value("device-uri"));
//     connectionLE->setProperty("orig_text", m_printer->value("device-uri"));
//     connect(nameLE, SIGNAL(textChanged(const QString &)),
//             this, SLOT(textChanged(const QString &)));
//     connect(locationLE, SIGNAL(textChanged(const QString &)),
//             this, SLOT(textChanged(const QString &)));
//     connect(connectionLE, SIGNAL(textChanged(const QString &)),
//             this, SLOT(textChanged(const QString &)));
}

PrinterBehavior::~PrinterBehavior()
{
}

void PrinterBehavior::setValues(const QHash<QString, QVariant> &values)
{
    foreach (const QString &value, values["printer-error-policy-supported"].value<QStringList>()) {
        errorPolicyCB->addItem(errorPolicyString(value), value);
    }
    QStringList errorPolicy = values["printer-error-policy"].value<QStringList>();
    if (!errorPolicy.isEmpty()) {
        errorPolicyCB->setCurrentIndex(errorPolicyCB->findData(errorPolicy.first()));
        errorPolicyCB->setProperty("printer-error-policy", errorPolicy.first());
    }

    foreach (const QString &value, values["printer-op-policy-supported"].value<QStringList>()) {
        operationPolicyCB->addItem(operationPolicyString(value), value);
    }
    QStringList operationPolicy = values["printer-op-policy"].value<QStringList>();
    if (!errorPolicy.isEmpty()) {
        operationPolicyCB->setCurrentIndex(operationPolicyCB->findData(operationPolicy.first()));
        operationPolicyCB->setProperty("printer-error-policy", errorPolicy.first());
    }

    foreach (const QString &value, values["job-sheets-supported"].value<QStringList>()) {
        startingBannerCB->addItem(jobSheetsString(value), value);
        endingBannerCB->addItem(jobSheetsString(value), value);
    }
    QStringList bannerPolicy = values["job-sheets-default"].value<QStringList>();
    if (bannerPolicy.size() == 2) {
        startingBannerCB->setCurrentIndex(startingBannerCB->findData(bannerPolicy.at(0)));
        startingBannerCB->setProperty("job-sheets-supported", bannerPolicy.at(0));
        endingBannerCB->setCurrentIndex(endingBannerCB->findData(bannerPolicy.at(1)));
        endingBannerCB->setProperty("job-sheets-supported", bannerPolicy.at(1));
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

void PrinterBehavior::textChanged(const QString &text)
{
//     QLineEdit *le = qobject_cast<QLineEdit *>(sender());
//     bool isDifferent = le->property("orig_text") != text;
//     if (isDifferent != le->property("different").toBool()) {
//         isDifferent ? m_changes++ : m_changes--;
//         le->setProperty("different", isDifferent);
//         emit changed(m_changes);
//     }
}

void PrinterBehavior::save()
{
//     if (m_changes) {
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
//     }
}

bool PrinterBehavior::hasChanges()
{
    return m_changes;
}

#include "PrinterBehavior.moc"
