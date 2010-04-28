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

#include "PrinterModel.h"

#include <QDateTime>
#include <QMimeData>
#include <KUser>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include "QCups.h"

using namespace QCups;

PrinterModel::PrinterModel(WId parentId, QObject *parent)
 : QStandardItemModel(parent),
   m_parentId(parentId)
{
    setHorizontalHeaderItem(0,    new QStandardItem(i18n("Printers")));
    update();
}

void PrinterModel::update()
{
    ReturnArguments dests;
    QStringList requestAttr;
    requestAttr << "printer-name"
                << "printer-state"
                << "printer-state-message"
                << "printer-is-shared"
                << "printer-type"
                << "printer-location"
                << "printer-info"
                << "printer-make-and-model"
                << "printer-commands"
                << "marker-change-time"
                << "marker-colors"
                << "marker-high-levels"
                << "marker-levels"
                << "marker-low-levels"
                << "marker-message"
                << "marker-names"
                << "marker-types";
    // Get destinations with these attributes
    Result *ret = QCups::getDests(-1, requestAttr);
    dests = ret->result();
    delete ret;

    for (int i = 0; i < dests.size(); i++) {
//         kDebug() << dests.at(i);
        // If there is a printer and it's not the current one add it
        // as a new destination
        int dest_row = destRow(dests.at(i)["printer-name"].toString());
        if (dest_row == -1) {
            // not found, insert new one
            insertDest(i, dests.at(i));
        } else if (dest_row == i) {
            // update the printer
            updateDest(item(i), dests.at(i));
        } else {
            // found at wrong position
            // take it and insert on the right position
            QList<QStandardItem *> row = takeRow(dest_row);
            insertRow(i, row);
            updateDest(item(i), dests.at(i));
        }
    }

    // remove old printers
    // The above code starts from 0 and make sure
    // dest == modelIndex(x) and if it's not the
    // case it either inserts or moves it.
    // so any item > num_jobs can be safely deleted
    while (rowCount() > dests.size()) {
        removeRow(rowCount() - 1);
    }
}

void PrinterModel::insertDest(int pos, const QCups::Destination &dest)
{
    // Create the printer item
    QStandardItem *stdItem = new QStandardItem(dest["printer-name"].toString());
    stdItem->setData(dest["printer-name"].toString(), DestName);
    stdItem->setIcon(KIcon("printer"));
    // update the item
    updateDest(stdItem, dest);

    // insert the printer Item
    insertRow(pos, stdItem);
}

void PrinterModel::updateDest(QStandardItem *destItem, const QCups::Destination &dest)
{
    // store if the printer is the network default
    bool isDefault = dest["printer-type"].toInt() & CUPS_PRINTER_DEFAULT;
    if (isDefault != destItem->data(DestIsDefault).toBool()) {
        destItem->setData(isDefault, DestIsDefault);
    }

    // store the printer state
    QString status = destStatus(dest["printer-state"].toInt(),
                                dest["printer-state-message"].toString());
    if (status != destItem->data(DestStatus)) {
        destItem->setData(status, DestStatus);
    }

    // store if the printer is shared
    bool shared = dest["printer-is-shared"].toBool();
    if (shared != destItem->data(DestIsShared)) {
        destItem->setData(shared, DestIsShared);
    }

    // store if the printer is a class
    // the printer-type param is a flag
    bool isClass = dest["printer-type"].toInt() & CUPS_PRINTER_CLASS;
    if (isClass != destItem->data(DestIsClass)) {
        destItem->setData(isClass, DestIsClass);
    }

    // store the printer location
    QString location = dest["printer-location"].toString();
    if (location != destItem->data(DestLocation).toString()) {
        destItem->setData(location, DestLocation);
    }

    if (destItem->data(DestName).toString() != destItem->text()){
        if (destItem->text() != destItem->data(DestName).toString()){
            destItem->setText(destItem->data(DestName).toString());
        }
    }

    // store the printer description
    QString description = dest["printer-info"].toString();
    if (description != destItem->data(DestDescription).toString()){
        destItem->setData(description, DestDescription);
    }

    // store the printer kind
    QString kind = dest["printer-make-and-model"].toString();
    if (kind != destItem->data(DestKind)) {
        destItem->setData(kind, DestKind);
    }

    // store the printer commands
    QStringList commands = dest["printer-commands"].toStringList();
    if (commands != destItem->data(DestCommands)) {
        destItem->setData(commands, DestCommands);
    }
}

int PrinterModel::destRow(const QString &destName)
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (destName == item(i)->data(DestName).toString())
        {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString PrinterModel::destStatus(int state, const QString &message) const
{
    switch (state) {
    case DEST_IDLE :
        if (message.isEmpty()){
            return i18n("Idle");
        } else {
            return i18n("Idle - '%1'", message);
        }
    case DEST_PRINTING :
        if (message.isEmpty()){
            return i18n("In use");
        } else {
            return i18n("In use - '%1'", message);
        }
    case DEST_STOPED :
        if (message.isEmpty()){
            return i18n("Paused");
        } else {
            return i18n("Paused - '%1'", message);
        }
    default :
        if (message.isEmpty()){
            return i18n("Unknown");
        } else {
            return i18n("Unknown - '%1'", message);
        }
    }
}

Qt::ItemFlags PrinterModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

#include "PrinterModel.moc"
