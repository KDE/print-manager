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

#ifndef SELECT_MAKE_MODEL_H
#define SELECT_MAKE_MODEL_H

#include <QWidget>
#include <QDBusMessage>

#include <kdemacros.h>

#include "KCupsConnection.h"
#include "PPDModel.h"

namespace Ui {
    class SelectMakeModel;
}

class KCupsRequest;
class KDE_EXPORT SelectMakeModel : public QWidget
{
    Q_OBJECT
public:
    explicit SelectMakeModel(QWidget *parent = 0);
    ~SelectMakeModel();

    void setDeviceInfo(const QString &deviceId, const QString &make, const QString &makeAndModel, const QString &deviceUri);
    void setMakeModel(const QString &make, const QString &makeAndModel);
    QString selectedPPDName() const;
    QString selectedPPDMakeAndModel() const;
    QString selectedPPDFileName() const;
    bool isFileSelected() const;

public slots:
    void checkChanged();
    void ppdsLoaded();

signals:
    void changed(bool);

private slots:
    void getBestDriversFinished(const QDBusMessage &message);
    void getBestDriversFailed(const QDBusError &error, const QDBusMessage &message);

private:
    void setModelData();
    void selectFirstMake();
    void selectMakeModelPPD();
    void selectRecommendedPPD();

    Ui::SelectMakeModel *ui;
    PPDModel *m_sourceModel;
    KCupsRequest *m_ppdRequest;
    ReturnArguments m_ppds;
    DriverMatchList m_driverMatchList;
    bool m_gotBestDrivers;
    bool m_hasRecommended;
    QString m_make;
    QString m_makeAndModel;
};

#endif
