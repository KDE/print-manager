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
    if (!driverMatch.isEmpty()) {
        recommended = new QStandardItem;
        recommended->setText(i18n("Recommended Drivers"));
    }

    foreach (const QVariantHash &ppd, ppds) {
        QString make = ppd["ppd-make"].toString();
        QString makeAndModel = ppd["ppd-make-and-model"].toString();
        QString naturalLanguage = ppd["ppd-natural-language"].toString();
        QString ppdName = ppd["ppd-name"].toString();

        QStandardItem *makeItem = findCreateMake(make);

        // Removes the Make part of the string
        if (makeAndModel.startsWith(make)) {
            makeAndModel.remove(0, make.size() + 1);
        }

        // Create the PPD
        QStandardItem *ppdItem = new QStandardItem;
        ppdItem->setText(makeAndModel.trimmed() %
                         QLatin1String(" (") %
                         naturalLanguage %
                         QLatin1Char(')'));
        ppdItem->setData(ppdName, PPDName);
        makeItem->appendRow(ppdItem);

        // Check if the driver is not the recommended one
        if (recommended) {
            foreach (const DriverMatch &driver, driverMatch) {
                if (ppdName == driver.ppd) {
                    // Create the PPD (TODO move to a func)
                    QStandardItem *ppdItem = new QStandardItem;
                    ppdItem->setText(makeAndModel.trimmed() %
                                     QLatin1String(" (") %
                                     naturalLanguage %
                                     QLatin1Char(')'));
                    ppdItem->setData(ppdName, PPDName);
                    recommended->appendRow(ppdItem);
                }
            }
        }
    }
}

//QVariant PPDModel::data(const QModelIndex &index, int role) const
//{
//    if (!index.isValid()) {
//        return QVariant();
//    }

//    int row = index.row();
//    switch (role) {
//    case Qt::DisplayRole:
//        if (m_make.isEmpty()) {
//            return QString("%1 (%2)")
//                .arg(m_ppds.at(row)["ppd-make-and-model"].toString())
//                .arg(m_ppds.at(row)["ppd-natural-language"].toString());
//        } else {
//            QString first = m_ppds.at(row)["ppd-make-and-model"].toString();
//            // We are only looking at printers of X brand, so remove the
//            // brand name from the list
//            if (first.startsWith(m_make + ' ', Qt::CaseInsensitive)) {
//                first.remove(0, m_make.size() + 1);
//            }
//            return QString("%1 (%2)")
//               .arg(first)
//               .arg(m_ppds.at(row)["ppd-natural-language"].toString());
//        }
//    case PPDMake:
//        return m_ppds.at(row)["ppd-make"].toString();
//    case PPDName:
//        return m_ppds.at(row)["ppd-name"].toString();
//    case PPDMakeAndModel:
//        return m_ppds.at(row)["ppd-make-and-model"].toString();
//    default:
//        return QVariant();
//    }
//}

//void PPDModel::setMake(const QString &make)
//{
//    m_make = make;
//}

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

//int PPDModel::rowCount(const QModelIndex &parent) const
//{
//    Q_UNUSED(parent)
//    return m_ppds.size();
//}

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
