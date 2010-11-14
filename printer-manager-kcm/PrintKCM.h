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

#ifndef PRINT_KCM_H
#define PRINT_KCM_H

#include <KCModule>
#include <KTitleWidget>
#include <QStackedLayout>

#include "ui_PrintKCM.h"

class PrinterModel;
class PrinterDescription;
class PrintKCM : public KCModule, public Ui::PrintKCM
{
Q_OBJECT

public:
    PrintKCM(QWidget *parent, const QVariantList &args);
    ~PrintKCM();

private slots:
    void update();
    void on_addPB_clicked();
    void on_removePB_clicked();
    void on_configurePrinterPB_clicked();
    void on_preferencesPB_clicked();

    void error(int lastError, const QString &errorTitle, const QString &errorMsg);

private:
    PrinterModel *m_model;
    QStackedLayout *m_stackedLayout;
    PrinterDescription *m_printerDesc;
    QWidget *m_noPrinter;
    QWidget *m_serverError;
    KTitleWidget *m_serverErrorW;
    int m_lastError;

    QAction *m_addAction;
    QAction *m_removeAction;
    QAction *m_configureAction;
};

#endif
