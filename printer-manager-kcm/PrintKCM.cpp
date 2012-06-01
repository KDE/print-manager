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

#include "ui_PrintKCM.h"
#include "ui_ServerError.h"

#include "PrinterModel.h"
#include "PrinterDelegate.h"
#include "PrinterDescription.h"
#include "SystemPreferences.h"

#include <KMessageBox>
#include <KGenericFactory>
#include <KAboutData>
#include <KIcon>

#include <QDBusMessage>
#include <QDBusConnection>
#include <KCupsRequest.h>

K_PLUGIN_FACTORY(PrintKCMFactory, registerPlugin<PrintKCM>();)
K_EXPORT_PLUGIN(PrintKCMFactory("kcm_print"))

PrintKCM::PrintKCM(QWidget *parent, const QVariantList &args) :
    KCModule(PrintKCMFactory::componentData(), parent, args),
    ui(new Ui::PrintKCM),
    serverErrorUi(new Ui::ServerError),
    m_lastError(-1) // Force the error to run on the first time
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
    setButtons(NoAdditionalButton);
    KGlobal::insertCatalog(QLatin1String("print-manager"));

    ui->setupUi(this);

    m_addMenu = new QMenu();
    m_addMenu->addAction(KIcon("printer"),
                         i18nc("@action:intoolbar","Printer"),
                         this, SLOT(on_addTB_clicked()));
    m_addMenu->addAction(KIcon("applications-other"),
                         i18nc("@action:intoolbar","Printer Class"),
                         this, SLOT(addClass()));
    ui->addTB->setIcon(KIcon("list-add"));
    ui->addTB->setToolTip(i18n("Add a new printer or a printer class"));
    ui->addTB->setMenu(m_addMenu);

    ui->removeTB->setIcon(KIcon("list-remove"));
    ui->removeTB->setToolTip(i18n("Remove Printer"));

    ui->configureTB->setIcon(KIcon("configure"));
    ui->configureTB->setToolTip(i18n("Configure Printer"));

    ui->systemPreferencesTB->setIcon(KIcon("configure"));
    ui->systemPreferencesTB->setToolTip(i18n("Configure the global preferences"));

    m_model = new PrinterModel(winId(), this);
    ui->printersTV->setModel(m_model);
    ui->printersTV->setItemDelegate(new PrinterDelegate(this));
    connect(ui->printersTV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(update()));
    connect(ui->printersTV->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(update()));
    connect(ui->printersTV->model(), SIGNAL(error(int,QString,QString)),
            this, SLOT(error(int,QString,QString)));

    // Create the PrinterDescription before we try to select a printer
    m_printerDesc = new PrinterDescription(ui->scrollAreaWidgetContents);
    m_printerDesc->hide();

    m_serverError = new QWidget(ui->scrollAreaWidgetContents);
    m_serverError->hide();
    serverErrorUi->setupUi(m_serverError);

    serverErrorUi->addPrinterBtn->setIcon(KIcon("list-add"));
    connect(serverErrorUi->addPrinterBtn, SIGNAL(clicked()), this, SLOT(addPrinter()));

    // the stacked layout allow us to chose which widget to show
    m_stackedLayout = new QStackedLayout(ui->scrollAreaWidgetContents);
    m_stackedLayout->addWidget(m_serverError);
    m_stackedLayout->addWidget(m_printerDesc);
    ui->scrollAreaWidgetContents->setLayout(m_stackedLayout);

    // Force the model update AFTER we setup the error signal
    m_model->update();

    // select the first printer if there are printers
    if (m_model->rowCount()) {
        ui->printersTV->selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::Select);
    }
}

void PrintKCM::error(int lastError, const QString &errorTitle, const QString &errorMsg)
{
    if (lastError) {
        // The user has no printer
        // allow him to add a new one
        if (lastError == IPP_NOT_FOUND) {
            noPrinters();
        } else {
            serverErrorUi->hugeIcon->setPixmap(KIcon("printer", KIconLoader::global(), QStringList() << "" << "dialog-error").pixmap(128, 128));
            serverErrorUi->errorText->setText(QString("<strong>%1</strong>").arg(errorTitle));
            serverErrorUi->errorComment->setText(errorMsg);
            serverErrorUi->errorComment->show();
            serverErrorUi->addPrinterBtn->hide();
        }

        if (m_stackedLayout->widget() != m_serverError) {
            m_stackedLayout->setCurrentWidget(m_serverError);
        }
    }

    if (m_lastError != lastError) {
        // if no printer was found the server
        // is still working
        if (lastError == IPP_NOT_FOUND) {
            ui->addTB->setEnabled(true);
            ui->systemPreferencesTB->setEnabled(true);
        } else {
            ui->addTB->setEnabled(!lastError);
            ui->systemPreferencesTB->setEnabled(!lastError);
        }

        ui->removeTB->setEnabled(false);
        ui->configureTB->setEnabled(false);
        ui->printersTV->setEnabled(!lastError);
        m_lastError = lastError;
        // Force an update
        update();
    }
}

void PrintKCM::noPrinters()
{
    serverErrorUi->hugeIcon->setPixmap(KIcon("dialog-information").pixmap(128, 128));
    serverErrorUi->errorText->setText(i18n("No printers have been configured or discovered"));
    serverErrorUi->errorComment->hide();
    serverErrorUi->addPrinterBtn->show();
}

PrintKCM::~PrintKCM()
{
    delete ui;
    delete serverErrorUi;
}

void PrintKCM::update()
{
    if (m_model->rowCount()) {
        if (m_stackedLayout->widget() != m_printerDesc) {
            m_stackedLayout->setCurrentWidget(m_printerDesc);
        }

        QItemSelection selection;
        // we need to map the selection to source to get the real indexes
        selection = ui->printersTV->selectionModel()->selection();
        // select the first printer if there are printers
        if (selection.indexes().isEmpty()) {
            ui->printersTV->selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::Select);
            return;
        }

        QModelIndex index = selection.indexes().first();
        QString destName = index.data(PrinterModel::DestName).toString();
        if (m_printerDesc->destName() != destName) {
            m_printerDesc->setPrinterIcon(index.data(Qt::DecorationRole).value<QIcon>());
            int type = index.data(PrinterModel::DestType).toInt();
            // If we remove discovered printers, they will come
            // back to hunt us a bit later
            ui->removeTB->setEnabled(!(type & CUPS_PRINTER_DISCOVERED));
            ui->configureTB->setEnabled(true);
        }
        m_printerDesc->setDestName(index.data(PrinterModel::DestName).toString(),
                                   index.data(PrinterModel::DestDescription).toString(),
                                   index.data(PrinterModel::DestIsClass).toBool());
        m_printerDesc->setLocation(index.data(PrinterModel::DestLocation).toString());
        m_printerDesc->setStatus(index.data(PrinterModel::DestStatus).toString());
        m_printerDesc->setKind(index.data(PrinterModel::DestKind).toString());
        m_printerDesc->setIsShared(index.data(PrinterModel::DestIsShared).toBool());
        m_printerDesc->setIsDefault(index.data(PrinterModel::DestIsDefault).toBool());
        m_printerDesc->setCommands(index.data(PrinterModel::DestCommands).toStringList());
        m_printerDesc->setMarkers(index.data(PrinterModel::DestMarkers).value<QVariantHash>());
    } else {
        // disable the printer action buttons if there is nothing to selected
        ui->removeTB->setEnabled(false);
        ui->configureTB->setEnabled(false);

        if (m_stackedLayout->widget() != m_serverError) {
            // the model is empty and no problem happened
            noPrinters();
            m_stackedLayout->setCurrentWidget(m_serverError);
        }
    }
}

void PrintKCM::on_addTB_clicked()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.AddPrinter",
                                             "/",
                                             "org.kde.AddPrinter",
                                             QLatin1String("AddPrinter"));
    message << qVariantFromValue(winId());
    QDBusConnection::sessionBus().call(message);
}

void PrintKCM::addClass()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.AddPrinter",
                                             "/",
                                             "org.kde.AddPrinter",
                                             QLatin1String("AddClass"));
    message << qVariantFromValue(winId());
    QDBusConnection::sessionBus().call(message);
}

void PrintKCM::on_removeTB_clicked()
{
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = ui->printersTV->selectionModel()->selection();
    // enable or disable the job action buttons if something is selected
    if (!selection.indexes().isEmpty()) {
        QModelIndex index = selection.indexes().first();
        int resp;
        QString msg, title;
        if (index.data(PrinterModel::DestIsClass).toBool()) {
            title = i18n("Remove class");
            msg = i18n("Are you sure you want to remove the class '%1'?",
                       index.data(Qt::DisplayRole).toString());
        } else {
            title = i18n("Remove printer");
            msg = i18n("Are you sure you want to remove the printer '%1'?",
                       index.data(Qt::DisplayRole).toString());
        }
        resp = KMessageBox::questionYesNo(this, msg, title);
        if (resp == KMessageBox::Yes) {
            KCupsRequest *request = new KCupsRequest;
            request->deletePrinter(index.data(PrinterModel::DestName).toString());
            request->waitTillFinished();
            request->deleteLater();
        }
    }
}

void PrintKCM::on_configureTB_clicked()
{
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = ui->printersTV->selectionModel()->selection();
    // enable or disable the job action buttons if something is selected
    if (!selection.indexes().isEmpty()) {
        QModelIndex index = selection.indexes().at(0);
        QDBusMessage message;
        message = QDBusMessage::createMethodCall("org.kde.ConfigurePrinter",
                                                 "/",
                                                 "org.kde.ConfigurePrinter",
                                                 QLatin1String("ConfigurePrinter"));
        message << qVariantFromValue(index.data(PrinterModel::DestName).toString());
        QDBusConnection::sessionBus().send(message);
    }
}

void PrintKCM::on_systemPreferencesTB_clicked()
{
    SystemPreferences *dlg = new SystemPreferences(this);
    dlg->show();
}
