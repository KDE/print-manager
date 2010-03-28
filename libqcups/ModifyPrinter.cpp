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

#include "QCups.h"

#include <KDebug>

using namespace QCups;

ModifyPrinter::ModifyPrinter(const QString &destName, bool isClass, QWidget *parent)
 : PrinterPage(parent), m_destName(destName), m_isClass(isClass), m_changes(0)
{
    setupUi(this);

    connectionL->setVisible(!isClass);
    connectionLE->setVisible(!isClass);

    membersL->setVisible(isClass);
    membersLV->setVisible(isClass);

    Printer *printer = new Printer(destName, this);

    nameLE->setText(printer->value("printer-info"));
    nameLE->setProperty("orig_text", printer->value("printer-info"));

    locationLE->setText(printer->value("printer-location"));
    locationLE->setProperty("orig_text", printer->value("printer-location"));

    connectionLE->setText(printer->value("device-uri"));
    connectionLE->setProperty("orig_text", printer->value("device-uri"));

    makeCB->addItem(printer->value("printer-make-and-model"));

    connect(nameLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(locationLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(connectionLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
}

ModifyPrinter::~ModifyPrinter()
{
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
        kDebug() << m_changedValues;
        QCups::Printer::setAttributes(m_destName, m_isClass, m_changedValues);
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

#include "ModifyPrinter.moc"
