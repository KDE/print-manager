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

#ifndef CONFIGURE_DIALOG_H
#define CONFIGURE_DIALOG_H

#include <QCloseEvent>
#include <KPageDialog>

class PrinterPage;
class ModifyPrinter;
class PrinterOptions;
class KDE_EXPORT ConfigureDialog : public KPageDialog
{
    Q_OBJECT
public:
    explicit ConfigureDialog(const QString &destName, bool isClass, QWidget *parent = 0);
    ~ConfigureDialog();

private slots:
    void currentPageChanged(KPageWidgetItem *current, KPageWidgetItem *before);
    virtual void slotButtonClicked(int button);
    void ppdChanged();

private:
    ModifyPrinter *modifyPrinter;
    PrinterOptions *printerOptions;
    void closeEvent(QCloseEvent *event);
    // return false if the dialog was canceled
    bool savePage(PrinterPage *page);
};

#endif
