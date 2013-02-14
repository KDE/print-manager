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

#ifndef PAGE_DESTINATIONS_H
#define PAGE_DESTINATIONS_H

#include "GenericPage.h"

#include <KCupsPrinter.h>
#include <KUrl>

#include <QLabel>

namespace Ui {
    class PageDestinations;
}

class DevicesModel;
class ChooseLpd;
class ChoosePrinters;
class ChooseSamba;
class ChooseSerial;
class ChooseSocket;
class ChooseUri;
class PageDestinations : public GenericPage
{
    Q_OBJECT
public:
    explicit PageDestinations(const QVariantHash &args = QVariantHash(), QWidget *parent = 0);
    ~PageDestinations();

    bool canProceed() const;
    void setValues(const QVariantHash &args);
    QVariantHash values() const;
    bool isValid() const;

private slots:
    void deviceChanged();
    void deviceUriChanged();
    void insertDevice(const QString &device_class,
                      const QString &device_id,
                      const QString &device_info,
                      const QString &device_make_and_model,
                      const QString &device_uri,
                      const QString &device_location,
                      const KCupsPrinters &grouped_printers);

private:
    QString uriText(const QString &uri) const;
    QVariantHash selectedItemValues() const;
    void setCurrentPage(QWidget *widget, const QVariantHash &args);

    Ui::PageDestinations *ui;
    DevicesModel *m_model;

    QString m_currentUri;
    ChooseLpd      *m_chooseLpd;
    ChoosePrinters *m_choosePrinters;
    ChooseSamba    *m_chooseSamba;
    ChooseSerial   *m_chooseSerial;
    ChooseSocket   *m_chooseSocket;
    ChooseUri      *m_chooseUri;
    QLabel         *m_chooseLabel;
};

#endif
