/*
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SELECTMAKEMODELDIALOG_H
#define SELECTMAKEMODELDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

#include "SelectMakeModel.h"

class SelectMakeModelDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectMakeModelDialog(const QString & make, const QString & makeModel, QWidget *parent = nullptr);
    ~SelectMakeModelDialog() override;

    SelectMakeModel * mainWidget() const;
private:
    SelectMakeModel * m_widget = nullptr;
    QDialogButtonBox * m_bbox = nullptr;
};

#endif // SELECTMAKEMODELDIALOG_H
