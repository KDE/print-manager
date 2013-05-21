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
#include <KConfigDialogManager>

ClassListWidget::ClassListWidget(QWidget *parent) :
    QListView(parent),
    m_request(0),
    m_showClasses(false)
{
    KConfigDialogManager::changedMap()->insert("ClassListWidget", SIGNAL(changed(QString)));

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

    m_delayedInit.setInterval(0);
    m_delayedInit.setSingleShot(true);
    connect(&m_delayedInit, SIGNAL(timeout()), SLOT(init()));
    m_delayedInit.start();
}

ClassListWidget::~ClassListWidget()
{
}

void ClassListWidget::init()
{
    m_busySeq->start(); // Start spining
    m_model->clear();

    QStringList att;
    att << KCUPS_PRINTER_NAME;
    att << KCUPS_PRINTER_URI_SUPPORTED;
    // Get destinations with these masks
    m_request = new KCupsRequest;
    connect(m_request, SIGNAL(finished()), this, SLOT(loadFinished()));
    if (m_showClasses) {
        m_request->getPrinters(att);
    } else {
        m_request->getPrinters(att,
                               CUPS_PRINTER_CLASS | CUPS_PRINTER_REMOTE | CUPS_PRINTER_IMPLICIT);
    }
}

void ClassListWidget::loadFinished()
{
    // If we have an old request running discard it's result and get a new one
    if (m_request != sender()) {
        sender()->deleteLater();
        return;
    }

    m_busySeq->stop(); // Stop spining

    KCupsPrinters printers = m_request->printers();
    m_request->deleteLater();
    m_request = 0;

    foreach (const KCupsPrinter &printer, printers) {
        QString destName = printer.name();
        if (destName != m_printerName) {
            QStandardItem *item = new QStandardItem;
            item->setText(destName);
            item->setCheckable(true);
            item->setEditable(false);
            item->setData(printer.uriSupported());
            updateItemState(item);

            m_model->appendRow(item);
        }
    }

    modelChanged();
}

void ClassListWidget::modelChanged()
{
    QStringList currentMembers = currentSelected(false);

    m_changed = m_selectedPrinters != currentMembers;

    emit changed(selectedPrinters());
    emit changed(m_changed);
}

QStringList ClassListWidget::currentSelected(bool uri) const
{
    QStringList currentMembers;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QStandardItem *item = m_model->item(i);
        if (item && item->checkState() == Qt::Checked) {
            if (uri) {
                currentMembers << item->data().toString();
            } else {
                currentMembers << item->text();
            }
        }
    }
    currentMembers.sort();
    return currentMembers;
}

void ClassListWidget::updateItemState(QStandardItem *item) const
{
    if (m_selectedPrinters.contains(item->text())) {
        item->setCheckState(Qt::Checked);
    } else {
        item->setCheckState(Qt::Unchecked);
    }
}

bool ClassListWidget::hasChanges()
{
    return m_changed;
}

void ClassListWidget::setPrinter(const QString &printer)
{
    if (m_printerName != printer) {
        m_printerName = printer;
        m_delayedInit.start();
    }
}

QString ClassListWidget::selectedPrinters() const
{
    return currentSelected(false).join(QLatin1String("|"));
}

void ClassListWidget::setSelectedPrinters(const QString &selected)
{
    m_selectedPrinters = selected.split(QLatin1Char('|'));
    m_selectedPrinters.sort();
    m_delayedInit.start();
}

bool ClassListWidget::showClasses() const
{
    return m_showClasses;
}

void ClassListWidget::setShowClasses(bool enable)
{
    if (m_showClasses != enable) {
        m_showClasses = enable;
        m_delayedInit.start();
    }
}

#include "ClassListWidget.moc"
