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

#ifndef PRINTER_BEHAVIOR_H
#define PRINTER_BEHAVIOR_H

#include "PrinterPage.h"

#include "KCupsRequest.h"
#include <QWidget>

namespace Ui {
    class PrinterBehavior;
}

class PrinterBehavior : public PrinterPage
{
    Q_OBJECT
public:
    explicit PrinterBehavior(const QString &destName, bool isClass, QWidget *parent = 0);
    ~PrinterBehavior();

    void setValues(const KCupsPrinter &printer);
    void setRemote(bool remote);
    bool hasChanges();

    QStringList neededValues() const;
    void save();

private slots:
    void currentIndexChangedCB(int index);
    void userListChanged();

private:
    QString errorPolicyString(const QString &policy) const;
    QString operationPolicyString(const QString &policy) const;
    QString jobSheetsString(const QString &policy) const;

    Ui::PrinterBehavior *ui;
    QString m_destName;
    bool m_isClass;
    QVariantHash m_changedValues;
    int m_changes;
};

#endif
