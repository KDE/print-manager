/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHOOSE_SERIAL_H
#define CHOOSE_SERIAL_H

#include "GenericPage.h"

#include <QRegExp>

namespace Ui {
    class ChooseSerial;
}

class ChooseSerial : public GenericPage
{
    Q_OBJECT
public:
    explicit ChooseSerial(QWidget *parent = nullptr);
    ~ChooseSerial() override;

    void setValues(const QVariantHash &args) override;
    QVariantHash values() const override;
    bool isValid() const override;

public slots:
    void load();

private:
    Ui::ChooseSerial *ui;
    QRegExp m_rx;
    bool m_isValid = false;
};

#endif
