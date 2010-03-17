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

#include "ConfigureDialog.h"
#include "PrinterPage.h"

#include "ModifyPrinter.h"

#include <KMessageBox>
#include <KDebug>

using namespace QCups;

ConfigureDialog::ConfigureDialog(const QString &destName, QWidget *parent)
 : KPageDialog(parent)
{
    setFaceType(List);
    setModal(true);
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)),
            SLOT(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)));
kDebug();
    ModifyPrinter *widget = new ModifyPrinter(destName, this);
    KPageWidgetItem *page = new KPageWidgetItem(widget, i18n("Modify Printer"));
    page->setHeader(i18n("Configure"));
    page->setIcon(KIcon("file"));
kDebug();
    addPage(page);
}

ConfigureDialog::~ConfigureDialog()
{
}

void ConfigureDialog::currentPageChanged(KPageWidgetItem *current, KPageWidgetItem *before)
{
    Q_UNUSED(before)
    kDebug();
    PrinterPage *page = qobject_cast<PrinterPage*>(current->widget());
    kDebug() << page;
    if (page->hasChanges()) {
        int ret;
        ret = KMessageBox::questionYesNoCancel(this,
                                               i18n("The current page has changes, "
                                                    "do you want to save?"));
        if (ret == KMessageBox::Yes) {
            page->save();
        }
    }
}

void ConfigureDialog::slotButtonClicked(int button)
{
    PrinterPage *page = qobject_cast<PrinterPage*>(currentPage());
    if (button == KDialog::Ok)
        accept();
    else
        KDialog::slotButtonClicked(button);
}

bool ConfigureDialog::savePage(PrinterPage *page)
{
    if (page->hasChanges()) {
        int ret;
        ret = KMessageBox::questionYesNoCancel(this,
                                               i18n("The current page has changes, "
                                                    "do you want to save?"));
        if (ret == KMessageBox::Yes) {
            page->save();
        } else if (ret == KMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

#include "ConfigureDialog.moc"
