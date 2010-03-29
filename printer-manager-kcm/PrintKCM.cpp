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

#include "PrintKCM.h"

#include "PrinterModel.h"
#include "PrinterDelegate.h"
#include "PrinterDescription.h"

#include <KMessageBox>
#include <KGenericFactory>
#include <KAboutData>
#include <KIcon>

#include <QTimer>
#include <QCups.h>

K_PLUGIN_FACTORY(PrintKCMFactory, registerPlugin<PrintKCM>();)
K_EXPORT_PLUGIN(PrintKCMFactory("kcm_print"))

PrintKCM::PrintKCM(QWidget *parent, const QVariantList &args)
    : KCModule(PrintKCMFactory::componentData(), parent, args)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kcm_print",
                               "kcm_print",
                               ki18n("Print settings"),
                               "0.1",
                               ki18n("Print settings"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2010 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(Help);

    setupUi(this);

    addPB->setIcon(KIcon("list-add"));
    removePB->setIcon(KIcon("list-remove"));
    preferencesPB->setIcon(KIcon("configure"));

    QCups::initialize();
    m_model = new PrinterModel(winId(), this);
    printersTV->setModel(m_model);
    printersTV->setItemDelegate(new PrinterDelegate(this));
    connect(printersTV->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(update()));
    connect(printersTV->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(update()));

    // setup the timer that updates the UIs
    m_updateT = new QTimer(this);
    m_updateT->setInterval(1000);
    m_updateT->start();
    connect(m_updateT, SIGNAL(timeout()),
            m_model, SLOT(update()));

    // Create the PrinterDescription before we try to select a printer
    m_printerDesc = new PrinterDescription(this);
    m_printerDesc->hide();

    // select the first printer if there are printers
    if (m_model->rowCount() > 0) {
        printersTV->selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::Select);
    }
}

PrintKCM::~PrintKCM()
{
    m_updateT->stop();
}

void PrintKCM::update()
{
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = printersTV->selectionModel()->selection();
    // enable or disable the job action buttons if something is selected
    if (!selection.indexes().isEmpty()) {
        if (scrollArea->widget() != m_printerDesc) {
            // always take the widget before setting a new one otherwise it will be deleted
            scrollArea->takeWidget();
            scrollArea->setWidget(m_printerDesc);
        }
        removePB->setEnabled(true);
        QModelIndex index = selection.indexes().at(0);
        m_printerDesc->setDestName(index.data(PrinterModel::DestName).toString(),
                                   index.data(PrinterModel::DestDescription).toString(),
                                   index.data(PrinterModel::DestIsClass).toBool());
        m_printerDesc->setLocation(index.data(PrinterModel::DestLocation).toString());
        m_printerDesc->setStatus(index.data(PrinterModel::DestStatus).toString());
        m_printerDesc->setKind(index.data(PrinterModel::DestKind).toString());
        m_printerDesc->setIsShared(index.data(PrinterModel::DestIsShared).toBool());
        m_printerDesc->setIsDefault(index.data(PrinterModel::DestIsDefault).toBool());
    } else if (!noPrinterL->isVisible()) {
        // always take the widget before setting a new one otherwise it will be deleted
        scrollArea->takeWidget();
        scrollArea->setWidget(noPrinterL);
        removePB->setEnabled(false);
    }
}

void PrintKCM::on_removePB_clicked()
{
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = printersTV->selectionModel()->selection();
    // enable or disable the job action buttons if something is selected
    if (!selection.indexes().isEmpty()) {
        QModelIndex index = selection.indexes().at(0);
        int resp;
        resp = KMessageBox::questionYesNo(this,
                                          i18n("Are you sure you want to remove the printer '%1'?",
                                               index.data(Qt::DisplayRole).toString()),
                                          i18n("Remove printer"));
        if (resp == KMessageBox::Yes) {
            QCups::deletePrinter(index.data(PrinterModel::DestName).toString());
        }
    }
}
