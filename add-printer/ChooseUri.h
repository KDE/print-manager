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

#ifndef CHOOSE_URI_H
#define CHOOSE_URI_H

#include "GenericPage.h"

#include <KCupsPrinter.h>

#include <QUrl>

namespace Ui {
    class ChooseUri;
}
class KCupsRequest;
class ChooseUri : public GenericPage
{
    Q_OBJECT
public:
    explicit ChooseUri(QWidget *parent = 0);
    ~ChooseUri();

    void setValues(const QVariantHash &args) Q_DECL_OVERRIDE;
    QVariantHash values() const Q_DECL_OVERRIDE;
    bool isValid() const Q_DECL_OVERRIDE;
    bool canProceed() const Q_DECL_OVERRIDE;

public slots:
    void load();

signals:
    void errorMessage(const QString &message);
    void insertDevice(const QString &device_class,
                      const QString &device_id,
                      const QString &device_info,
                      const QString &device_make_and_model,
                      const QString &device_uri,
                      const QString &device_location,
                      const KCupsPrinters &grouped_printers);

private slots:
    void checkSelected();
    void on_addressLE_textChanged(const QString &text);
    void findPrinters();
    void getPrintersFinished(KCupsRequest *request);

private:
    QUrl parsedURL(const QString &text) const;

    Ui::ChooseUri *ui;
};

#endif
