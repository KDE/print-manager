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

#include "ClassListWidget.h"

#include "SelectMakeModel.h"

#include "KCupsRequest.h"
#include "NoSelectionRectDelegate.h"

#include <QPointer>
#include <KFileDialog>
#include <KDebug>

#include <KPixmapSequence>

ClassListWidget::ClassListWidget(QWidget *parent) :
    QListView(parent),
    m_request(0)
{
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    setItemDelegate(new NoSelectionRectDelegate(this));

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(viewport());

    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
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

    QStringList att;
    att << KCUPS_PRINTER_NAME;
    att << KCUPS_PRINTER_URI_SUPPORTED;
    // Get destinations with these masks
    m_request = new KCupsRequest;
    connect(m_request, SIGNAL(finished()), this, SLOT(loadFinished()));
    m_request->setProperty("reqDestName", reqDestName);
    m_request->setProperty("memberNames", memberNames);
    m_request->getPrinters(att,
                           CUPS_PRINTER_CLASS | CUPS_PRINTER_REMOTE | CUPS_PRINTER_IMPLICIT);

    m_busySeq->start(); // Start spining
}

void ClassListWidget::loadFinished()
{
    m_busySeq->stop(); // Stop spining

    KCupsPrinters printers;
    QString reqDestName;
    QStringList memberNames;
    printers    = m_request->printers();
    reqDestName = m_request->property("reqDestName").toString();
    memberNames = m_request->property("memberNames").toStringList();
    m_request->deleteLater();
    m_request = 0;

    m_model->clear();
    QStringList origMemberUris;
    foreach (const QString &memberUri, memberNames) {
        foreach (const KCupsPrinter &printer, printers) {
            if (printer.name() == memberUri) {
                origMemberUris << printer.uriSupported();
                break;
            }
        }
    }
    m_model->setProperty("orig-member-uris", origMemberUris);
    m_selectedDests = origMemberUris;

    foreach (const KCupsPrinter &printer, printers) {
        QString destName = printer.name();
        if (destName != reqDestName) {
            QStandardItem *item = new QStandardItem(destName);
            item->setCheckable(true);
            item->setEditable(false);
            if (memberNames.contains(destName)) {
                item->setCheckState(Qt::Checked);
            }
            item->setData(printer.uriSupported());
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

    // store the new values
    m_selectedDests = currentMembers;

    m_changed = m_model->property("orig-member-uris").toStringList() != currentMembers;
    emit changed(m_changed);
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
