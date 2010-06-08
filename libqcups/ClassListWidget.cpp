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

#include "ClassListWidget.h"

#include "SelectMakeModel.h"

#include "QCups.h"
#include <cups/cups.h>

#include <QPointer>
#include <KFileDialog>
#include <KDebug>

using namespace QCups;

ClassListWidget::ClassListWidget(QWidget *parent)
 : QListView(parent)
{
    m_model = new QStandardItemModel(this);
    setModel(m_model);

    connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(modelChanged()));
}

ClassListWidget::~ClassListWidget()
{
}

void ClassListWidget::reload(const QString &m_destName, const QStringList &memberNames)
{
    ReturnArguments dests;
    // Ask just these attributes
    QStringList requestAttr;
    requestAttr << "printer-uri-supported"
                << "printer-name";
    // Get destinations with these masks
    Result *ret = QCups::getDests(CUPS_PRINTER_CLASS | CUPS_PRINTER_REMOTE |
                                    CUPS_PRINTER_IMPLICIT, requestAttr);
    ret->waitTillFinished();
    dests = ret->result();
    ret->deleteLater();

    m_model->clear();
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
    m_selectedDests = origMemberUris;

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

    // clear old values
    m_changed = false;
}

void ClassListWidget::modelChanged()
{
    QStringList currentMembers;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QStandardItem *item = m_model->item(i);
        if (item && item->checkState() == Qt::Checked) {
            currentMembers << item->data().toString();
        }
    }
    currentMembers.sort();

    m_changed = m_model->property("orig-member-uris").toStringList() != currentMembers;
    emit changed(m_changed);

    // store the new values
    m_selectedDests = currentMembers;
}

bool ClassListWidget::hasChanges()
{
    return m_changed;
}

QStringList ClassListWidget::selectedDests() const
{
    return m_selectedDests;
}

#include "ClassListWidget.moc"
