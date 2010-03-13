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
#include "PrinterDescription.h"

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

    // setup the timer that updates the UIs
    QTimer *updateT = new QTimer(this);
    updateT->setInterval(1000);
    updateT->start();
    connect(updateT, SIGNAL(timeout()),
            this, SLOT(update()));

    QCups::initialize();
    m_model = new PrinterModel(winId(), this);
    printersTV->setModel(m_model);
    connect(printersTV->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(update()));

    m_printerDesc = new PrinterDescription;
    m_printerDesc->hide();
}

PrintKCM::~PrintKCM()
{
    delete m_printerDesc;
}

void PrintKCM::update()
{
    m_model->update();
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = printersTV->selectionModel()->selection();
    // enable or disable the job action buttons if something is selected
    if (!selection.indexes().isEmpty()) {
        if (scrollArea->widget() != m_printerDesc) {
            scrollArea->setWidget(m_printerDesc);
        }
        removePB->setEnabled(true);
        QModelIndex index = selection.indexes().at(0);
        m_printerDesc->setDestName(index.data(PrinterModel::DestName).toString());
        m_printerDesc->setLocation(index.data(PrinterModel::DestLocation).toString());
        m_printerDesc->setStatus(index.data(PrinterModel::DestStatus).toString());
        m_printerDesc->setDescription(index.data(PrinterModel::DestDescription).toString());
        m_printerDesc->setKind(index.data(PrinterModel::DestKind).toString());
        m_printerDesc->setIsShared(index.data(PrinterModel::DestIsShared).toBool());
        m_printerDesc->setIsDefault(index.data(PrinterModel::DestIsDefault).toBool());
    } else if (!noPrinterL->isVisible()) {
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
        QCups::deletePrinter(index.data(PrinterModel::DestName).toString());
    }
}
