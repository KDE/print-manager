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

#include <cups/cups.h>

#include <QPointer>
#include <KFileDialog>
#include <KDebug>

#include <KPixmapSequence>

using namespace QCups;

ClassListWidget::ClassListWidget(QWidget *parent)
 : QListView(parent),
   m_request(0)
{
    m_model = new QStandardItemModel(this);
    setModel(m_model);

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(viewport());

    connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(modelChanged()));
}

ClassListWidget::~ClassListWidget()
{
}

void ClassListWidget::reload(const QString &reqDestName, const QStringList &memberNames)
{
    // If we have an old request running discard it's result and get a new one
    if (m_request) {
        connect(m_request, SIGNAL(finished()), this, SLOT(deleteLater()));
        disconnect(m_request, SIGNAL(finished()), this, SLOT(loadFinished()));
    }

    // Ask just these attributes
    QStringList requestAttr;
    requestAttr << "printer-uri-supported"
                << "printer-name";

    // Get destinations with these masks
    m_request = QCups::getDests(CUPS_PRINTER_CLASS | CUPS_PRINTER_REMOTE |
                                    CUPS_PRINTER_IMPLICIT, requestAttr);
    m_request->setProperty("reqDestName", reqDestName);
    m_request->setProperty("memberNames", memberNames);
    connect(m_request, SIGNAL(finished()), this, SLOT(loadFinished()));

    m_busySeq->start(); // Start spining
}

void ClassListWidget::loadFinished()
{
    m_busySeq->stop(); // Stop spining

    ReturnArguments dests;
    QString reqDestName;
    QStringList memberNames;
    dests       = m_request->result();
    reqDestName = m_request->property("reqDestName").toString();
    memberNames = m_request->property("memberNames").toStringList();
    m_request->deleteLater();
    m_request = 0;

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
        if (destName != reqDestName) {
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
