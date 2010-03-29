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

PrinterModel::PrinterModel(WId parentId, QObject *parent)
 : QStandardItemModel(parent),
   m_parentId(parentId)
{
    setHorizontalHeaderItem(0,    new QStandardItem(i18n("Printers")));
    update();
}

void PrinterModel::update()
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest;
    int i;

    for (i = 0, dest = dests; i < num_dests; i++, dest++) {
        // If there is a printer and it's not the current one add it
        // as a new destination
        int dest_row = destRow(QString::fromLocal8Bit(dest->name));
        if (dest_row == -1) {
            // not found, insert new one
            insertDest(i, dest);
        } else if (dest_row == i) {
            // update the printer
            updateDest(item(i), dest);
        } else {
            // found at wrong position
            // take it and insert on the right position
            QList<QStandardItem *> row = takeRow(dest_row);
            insertRow(i, row);
            updateDest(item(i), dest);
        }
    }

    // remove old printers
    // The above code starts from 0 and make sure
    // dest == modelIndex(x) and if it's not the
    // case it either inserts or moves it.
    // so any item > num_jobs can be safely deleted
    while (rowCount() > num_dests) {
        removeRow(rowCount() - 1);
    }

    // don't leak
    cupsFreeDests(num_dests, dests);
}

void PrinterModel::insertDest(int pos, cups_dest_t *dest)
{
    // Create the printer item
    QStandardItem *stdItem = new QStandardItem(QString::fromLocal8Bit(dest->name));
    stdItem->setData(QString::fromLocal8Bit(dest->name), DestName);
    stdItem->setIcon(KIcon("printer"));
    // update the item
    updateDest(stdItem, dest);

    // insert the printer Item
    insertRow(pos, stdItem);
}

void PrinterModel::updateDest(QStandardItem *destItem, cups_dest_t *dest)
{
    const char *value;
    // store the default value
    bool isDefault = static_cast<bool>(dest->is_default);
    if (isDefault != destItem->data(DestIsDefault).toBool()) {
        destItem->setData(isDefault, DestIsDefault);
    }

    // store the printer state
    value = cupsGetOption("printer-state", dest->num_options, dest->options);
    if (value) {
        QString status = destStatus(value[0]);
        if (status != destItem->data(DestStatus)){
            destItem->setData(status, DestStatus);
        }
    }

    // store if the printer is shared
    value = cupsGetOption("printer-is-shared", dest->num_options, dest->options);
    if (value) {
        // Here we have a cups docs bug where the SHARED returned
        // value is the string "true" or "false", and not '1' or '0'
        bool shared = value[0] == 't' || value[0] == '1';
        if (shared != destItem->data(DestIsShared)) {
            destItem->setData(shared, DestIsShared);
        }
    }

    // store if the printer is a class
    value = cupsGetOption("printer-type", dest->num_options, dest->options);
    if (value) {
        // the printer-type param is a flag
        bool isClass = QString::fromLocal8Bit(value).toInt() & CUPS_PRINTER_CLASS;
        if (isClass != destItem->data(DestIsClass)) {
            destItem->setData(isClass, DestIsClass);
        }
    }

    // store the printer location
    value = cupsGetOption("printer-location", dest->num_options, dest->options);
    if (value) {
        QString location = QString::fromLocal8Bit(value);
        if (location != destItem->data(DestLocation).toString()) {
            destItem->setData(location, DestLocation);
        }
    }

    if (destItem->data(DestName).toString() != destItem->text()){
        if (destItem->text() != destItem->data(DestName).toString()){
            destItem->setText(destItem->data(DestName).toString());
        }
    }

    // store the printer description
    value = cupsGetOption("printer-info", dest->num_options, dest->options);
    if (value) {
        QString description = QString::fromLocal8Bit(value);
        if (description != destItem->data(DestDescription).toString()){
            destItem->setData(description, DestDescription);
        }
    }

    // store the printer kind
    value = cupsGetOption("printer-make-and-model", dest->num_options, dest->options);
    if (value) {
        QString kind = QString::fromLocal8Bit(value);
        if (kind != destItem->data(DestKind)) {
            destItem->setData(kind, DestKind);
        }
    }
}

int PrinterModel::destRow(const QString &destName)
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (destName == item(i)->data(DestName).toString().toLocal8Bit())
        {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString PrinterModel::destStatus(const char &state) const
{
    switch (state) {
    case DEST_IDLE :
        return i18n("Idle");
    case DEST_PRINTING :
        return i18n("In use");
    case DEST_STOPED :
        return i18n("Paused");
    default :
        return i18n("Unknown");
    }
}

Qt::ItemFlags PrinterModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

#include "PrinterModel.moc"
