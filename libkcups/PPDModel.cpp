/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
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

#include "PPDModel.h"

#include <QStringBuilder>

#include <KLocale>

#include <KDebug>

PPDModel::PPDModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

void PPDModel::setPPDs(const QList<QVariantHash> &ppds, const DriverMatchList &driverMatch)
{
    clear();

    QStandardItem *recommended = 0;
    foreach (const DriverMatch &driver, driverMatch) {
        // Find the matched PPD on the PPDs list
        foreach (const QVariantHash &ppd, ppds) {
            if (ppd["ppd-name"].toString() == driver.ppd) {
                // Create the PPD
                QStandardItem *ppdItem = createPPDItem(ppd, true);

                if (recommended == 0) {
                    recommended = new QStandardItem;
                    recommended->setText(i18n("Recommended Drivers"));
                    appendRow(recommended);
                }
                recommended->appendRow(ppdItem);

                break;
            }
        }
    }

    foreach (const QVariantHash &ppd, ppds) {
        // Find or create the PPD parent (printer Make)
        QStandardItem *makeItem = findCreateMake(ppd["ppd-make"].toString());

        // Create the PPD
        QStandardItem *ppdItem = createPPDItem(ppd, false);
        makeItem->appendRow(ppdItem);
    }
}

QStandardItem* PPDModel::findCreateMake(const QString &make)
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *makeItem = item(i);
        if (makeItem->text() == make) {
            return makeItem;
        }
    }

    QStandardItem *makeItem = new QStandardItem(make);
    appendRow(makeItem);
    return makeItem;
}

Qt::ItemFlags PPDModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void PPDModel::clear()
{
    // Remove all rows from the model
    removeRows(0, rowCount());
}

QStandardItem *PPDModel::createPPDItem(const QVariantHash &ppd, bool recommended)
{
    QStandardItem *ret = new QStandardItem;

    QString make = ppd["ppd-make"].toString();
    QString makeAndModel = ppd["ppd-make-and-model"].toString();
    QString naturalLanguage = ppd["ppd-natural-language"].toString();
    QString ppdName = ppd["ppd-name"].toString();

    // Set this data before we change the makeAndModel
    ret->setData(ppdName, PPDName);
    ret->setData(make, PPDMake);
    ret->setData(makeAndModel, PPDMakeAndModel);

    QString text;
    if (recommended) {
        text = makeAndModel %
                QLatin1String(" (") %
                naturalLanguage %
                QLatin1Char(')');
    } else {
        // Removes the Make part of the string
        if (makeAndModel.startsWith(make)) {
            makeAndModel.remove(0, make.size() + 1);
        }

        // Create the PPD
        text = makeAndModel.trimmed() %
                QLatin1String(" (") %
                naturalLanguage %
                QLatin1Char(')');
    }
    ret->setText(text);

    return ret;
}
