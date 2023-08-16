/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PPDModel.h"

#include "Debug.h"

#include <KLocalizedString>

PPDModel::PPDModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

void PPDModel::setPPDs(const QList<QVariantHash> &ppds, const DriverMatchList &driverMatch)
{
    clear();

    QStandardItem *recommended = nullptr;
    for (const DriverMatch &driver : driverMatch) {
        // Find the matched PPD on the PPDs list
        for (const QVariantHash &ppd : ppds) {
            if (ppd[QLatin1String("ppd-name")].toString() == driver.ppd) {
                // Create the PPD
                QStandardItem *ppdItem = createPPDItem(ppd, true);

                if (recommended == nullptr) {
                    recommended = new QStandardItem;
                    recommended->setText(i18n("Recommended Drivers"));
                    appendRow(recommended);
                }
                recommended->appendRow(ppdItem);

                break;
            }
        }
    }

    for (const QVariantHash &ppd : ppds) {
        // Find or create the PPD parent (printer Make)
        QStandardItem *makeItem = findCreateMake(ppd[QLatin1String("ppd-make")].toString());

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

    auto makeItem = new QStandardItem(make);
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
    auto ret = new QStandardItem;

    QString make = ppd[QLatin1String("ppd-make")].toString();
    QString makeAndModel = ppd[QLatin1String("ppd-make-and-model")].toString();
    QString naturalLanguage = ppd[QLatin1String("ppd-natural-language")].toString();
    QString ppdName = ppd[QLatin1String("ppd-name")].toString();

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

#include "moc_PPDModel.cpp"
