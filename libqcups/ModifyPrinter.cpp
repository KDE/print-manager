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

#include <QCups.h>

#include <KDebug>

using namespace QCups;

ModifyPrinter::ModifyPrinter(const QString &destName, QWidget *parent)
 : PrinterPage(parent), m_changes(0)
{
    setupUi(this);

    m_printer = new Printer(destName, this);

    makeCB->addItem(m_printer->value("printer-make-and-model"));
    nameLE->setText(m_printer->value("printer-info"));
    nameLE->setProperty("orig_text", m_printer->value("printer-info"));
    locationLE->setText(m_printer->value("printer-location"));
    locationLE->setProperty("orig_text", m_printer->value("printer-location"));
    connectionLE->setText(m_printer->value("device-uri"));
    connectionLE->setProperty("orig_text", m_printer->value("device-uri"));
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
    QLineEdit *le = qobject_cast<QLineEdit *>(sender());
    bool isDifferent = le->property("orig_text") != text;
    if (isDifferent != le->property("different").toBool()) {
        isDifferent ? m_changes++ : m_changes--;
        le->setProperty("different", isDifferent);
        emit changed(m_changes);
    }
}

void ModifyPrinter::save()
{
    if (m_changes) {
        QHash<QString, QVariant> values;
        if (nameLE->property("different").toBool()) {
            values["printer-info"] = nameLE->text();
        }
        if (locationLE->property("different").toBool()) {
            values["printer-location"] = locationLE->text();
        }
        if (connectionLE->property("different").toBool()) {
            values["device-uri"] = connectionLE->text();
        }
        m_printer->save(values);
    }
}

bool ModifyPrinter::hasChanges()
{
    return m_changes;
}

#include "ModifyPrinter.moc"
