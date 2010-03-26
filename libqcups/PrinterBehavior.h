/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#ifndef PRINTER_BEHAVIOR_H
#define PRINTER_BEHAVIOR_H

#include "ui_PrinterBehavior.h"

#include "PrinterPage.h"

#include "QCups.h"
#include <QWidget>

namespace QCups {

class PrinterBehavior : public PrinterPage, Ui::PrinterBehavior
{
    Q_OBJECT
public:
    explicit PrinterBehavior(const QString &destName, QWidget *parent = 0);
    ~PrinterBehavior();

    void setValues(const QHash<QString, QVariant> &values);
    bool hasChanges();

public:
    void save();

private slots:
    void currentIndexChangedCB(int index);
    void userListChanged();

private:
    QString m_destName;
    QHash<QString, QVariant> m_changedValues;
    int m_changes;

    QString errorPolicyString(const QString &policy) const;
    QString operationPolicyString(const QString &policy) const;
    QString jobSheetsString(const QString &policy) const;
};


}

#endif
