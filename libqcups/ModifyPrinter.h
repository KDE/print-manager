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

#ifndef MODIFY_PRINTER_H
#define MODIFY_PRINTER_H

#include "ui_ModifyPrinter.h"

#include "PrinterPage.h"

#include <QStandardItemModel>
#include <QWidget>

namespace QCups {

class ModifyPrinter : public PrinterPage, Ui::ModifyPrinter
{
    Q_OBJECT
    Q_ENUMS(Role)
public:
    typedef enum {
        PPDDefault,
        PPDCustom,
        PPDFile,
        PPDList,
        PPDName = Qt::UserRole + 1
    } Role;
    explicit ModifyPrinter(const QString &destName, bool isClass, QWidget *parent = 0);
    ~ModifyPrinter();

    bool hasChanges();
    QHash<QString, QVariant> modifiedValues() const;
    QStringList neededValues() const;
    void setRemote(bool remote);

    void setValues(const QHash<QString, QVariant> &values);
    void setCurrentMake(const QString &make);
    void setCurrentMakeAndModel(const QString &makeAndModel);

    void save();

private slots:
    void textChanged(const QString &text);
    void on_makeCB_activated(int index);
    void modelChanged();

private:
    QString m_destName, m_make, m_makeAndModel;
    bool m_isClass;
    QHash<QString, QVariant> m_changedValues;
    int m_changes;
    QStandardItemModel *m_model;
};


}

#endif
