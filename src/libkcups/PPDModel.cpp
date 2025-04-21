/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023-2025 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PPDModel.h"
#include "kcupslib_log.h"

#include <KCupsRequest.h>
#include <KLocalizedString>

PPDModel::PPDModel(QObject *parent)
    : QStandardItemModel(parent)
{
    m_roles = QStandardItemModel::roleNames();
    m_roles[PPDName] = "ppdName";
    m_roles[PPDMake] = "ppdMake";
    m_roles[PPDMakeAndModel] = "ppdMakeModel";
}

QHash<int, QByteArray> PPDModel::roleNames() const
{
    return m_roles;
}

QStandardItem *PPDModel::findCreateMake(const QString &make)
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

void PPDModel::load()
{
    qCDebug(LIBKCUPS) << "LOADING PPD Model";

    const auto req = new KCupsRequest;
    connect(req, &KCupsRequest::finished, this, [this](KCupsRequest *request) {
        clear();
        if (request->hasError()) {
            Q_EMIT error(request->errorMsg());
            qCDebug(LIBKCUPS) << "PPD Model:" << request->errorMsg();
        } else {
            const auto ppds = request->ppds();
            if (!ppds.isEmpty()) {
                for (const QVariantMap &ppd : ppds) {
                    // Find or create the PPD parent (printer Make)
                    QStandardItem *makeItem = findCreateMake(ppd[QLatin1String("ppd-make")].toString());

                    // Create the PPD item
                    auto ppdItem = new QStandardItem;

                    QString make = ppd[QLatin1String("ppd-make")].toString();
                    QString makeAndModel = ppd[QLatin1String("ppd-make-and-model")].toString();
                    QString naturalLanguage = ppd[QLatin1String("ppd-natural-language")].toString();
                    QString ppdName = ppd[QLatin1String("ppd-name")].toString();

                    // Set this data before we change the makeAndModel
                    ppdItem->setData(ppdName, PPDName);
                    ppdItem->setData(make, PPDMake);
                    ppdItem->setData(makeAndModel, PPDMakeAndModel);

                    // Removes the Make part of the string
                    if (makeAndModel.startsWith(make)) {
                        makeAndModel.remove(0, make.size() + 1);
                    }
                    ppdItem->setText(makeAndModel.trimmed() % QLatin1String(" (") % naturalLanguage % QLatin1Char(')'));
                    makeItem->appendRow(ppdItem);
                }
            } else {
                Q_EMIT error(i18n("Empty ppd model"));
            }
            qCDebug(LIBKCUPS) << "PPD Model Loaded";
        }
        request->deleteLater();
        Q_EMIT loaded();
    });

    req->getPPDS();
}

#include "moc_PPDModel.cpp"
