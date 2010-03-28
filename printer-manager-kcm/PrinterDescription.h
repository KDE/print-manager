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

#ifndef PRINTER_DESCRIPTION_H
#define PRINTER_DESCRIPTION_H

#include "ui_PrinterDescription.h"

class QToolButton;
class QSortFilterProxyModel;

class PrintQueueModel;

class PrinterDescription : public QWidget, Ui::PrinterDescription
{
    Q_OBJECT
public:
    explicit PrinterDescription(QWidget *parent = 0);
    ~PrinterDescription();

    void setDestName(const QString &name, bool isClass);
    void setLocation(const QString &location);
    void setStatus(const QString &status);
    void setDescription(const QString &description);
    void setKind(const QString &kind);
    void setIsDefault(bool isDefault);
    void setIsShared(bool isShared);

private slots:
    void on_openQueuePB_clicked();
    void on_defaultCB_clicked();
    void on_sharedCB_clicked();
    void on_optionsPB_clicked();

private:
    QString m_destName;
    bool m_isClass;
    QPixmap m_printerIcon;
    QPixmap m_pauseIcon;
    QPixmap m_startIcon;
    QPixmap m_warningIcon;
};

#endif
