/*
    SPDX-FileCopyrightText: 2010-2012 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAGE_DESTINATIONS_H
#define PAGE_DESTINATIONS_H

#include "GenericPage.h"

#include <KCupsPrinter.h>

#include <QUrl>
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
    explicit PageDestinations(const QVariantHash &args = QVariantHash(), QWidget *parent = nullptr);
    ~PageDestinations() override;

    bool canProceed() const override;
    void setValues(const QVariantHash &args) override;
    QVariantHash values() const override;
    bool isValid() const override;

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
