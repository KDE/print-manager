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

#include "PrinterModel.h"
#include "PrinterDelegate.h"
#include "PrinterDescription.h"

#include <KMessageBox>
#include <KGenericFactory>
#include <KAboutData>
#include <KIcon>

#include <QMenu>
#include <QDBusMessage>
#include <QDBusConnection>
#include <KCupsRequest.h>
#include <KCupsServer.h>

K_PLUGIN_FACTORY(PrintKCMFactory, registerPlugin<PrintKCM>();)
K_EXPORT_PLUGIN(PrintKCMFactory("kcm_print"))

PrintKCM::PrintKCM(QWidget *parent, const QVariantList &args) :
    KCModule(PrintKCMFactory::componentData(), parent, args),
    ui(new Ui::PrintKCM),
    m_lastError(-1) // Force the error to run on the first time
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kcm_print",
                               "kcm_print",
                               ki18n("Print settings"),
                               "0.1",
                               ki18n("Print settings"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2010-2012 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(NoAdditionalButton);

    ui->setupUi(this);

    QMenu *addMenu = new QMenu(this);
    addMenu->addAction(i18nc("@action:intoolbar","Add a Printer Class"),
                       this, SLOT(addClass()));
    ui->addTB->setIcon(KIcon("list-add"));
    ui->addTB->setToolTip(i18n("Add a new printer or a printer class"));
    ui->addTB->setMenu(addMenu);

    ui->removeTB->setIcon(KIcon("list-remove"));
    ui->removeTB->setToolTip(i18n("Remove Printer"));

    QMenu *systemMenu = new QMenu(this);
    connect(systemMenu, SIGNAL(triggered(QAction*)), this, SLOT(systemPreferencesTriggered()));
    m_showSharedPrinters = systemMenu->addAction(i18nc("@action:intoolbar","Show printers shared by other systems"));
    m_showSharedPrinters->setCheckable(true);
    systemMenu->addSeparator();
    m_shareConnectedPrinters = systemMenu->addAction(i18nc("@action:intoolbar","Share printers connected to this system"));
    m_shareConnectedPrinters->setCheckable(true);
    m_allowPrintringFromInternet = systemMenu->addAction(i18nc("@action:intoolbar","Allow printing from the Internet"));
    m_allowPrintringFromInternet->setCheckable(true);
    m_allowPrintringFromInternet->setEnabled(false);
    connect(m_shareConnectedPrinters, SIGNAL(toggled(bool)), m_allowPrintringFromInternet, SLOT(setEnabled(bool)));
    systemMenu->addSeparator();
    m_allowRemoteAdmin = systemMenu->addAction(i18nc("@action:intoolbar","Allow remote administration"));
    m_allowRemoteAdmin->setCheckable(true);
    m_allowUsersCancelAnyJob = systemMenu->addAction(i18nc("@action:intoolbar","Allow users to cancel any job (not just their own)"));
    m_allowUsersCancelAnyJob->setCheckable(true);

    ui->systemPreferencesTB->setIcon(KIcon("configure"));
    ui->systemPreferencesTB->setToolTip(i18n("Configure the global preferences"));
    ui->systemPreferencesTB->setMenu(systemMenu);

    m_model = new PrinterModel(winId(), this);
    ui->printersTV->setModel(m_model);
    ui->printersTV->setItemDelegate(new PrinterDelegate(this));
    connect(ui->printersTV->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(update()));
    connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(update()));
    connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(update()));
    connect(ui->printersTV->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(update()));
    connect(ui->printersTV->model(), SIGNAL(error(int,QString,QString)),
            this, SLOT(error(int,QString,QString)));

    ui->addPrinterBtn->setIcon(KIcon("list-add"));
    connect(ui->addPrinterBtn, SIGNAL(clicked()), this, SLOT(on_addTB_clicked()));

    // Force the model update AFTER we setup the error signal
    m_model->update();
}


PrintKCM::~PrintKCM()
{
    delete ui;
}

void PrintKCM::error(int lastError, const QString &errorTitle, const QString &errorMsg)
{
    if (lastError) {
        // The user has no printer
        // allow him to add a new one
        if (lastError == IPP_NOT_FOUND) {
            noPrinters();
        } else {
            ui->hugeIcon->setPixmap(KIcon("printer", KIconLoader::global(), QStringList() << "" << "dialog-error").pixmap(128, 128));
            ui->errorText->setText(QString("<strong>%1</strong>").arg(errorTitle));
            ui->errorComment->setText(errorMsg);
            ui->errorComment->show();
            ui->addPrinterBtn->hide();
        }

        // 1 is the error message
        if (ui->stackedWidget->currentIndex() != 1) {
            ui->stackedWidget->setCurrentIndex(1);
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

        m_lastError = lastError;
        // Force an update
        update();
    }
}

void PrintKCM::noPrinters()
{
    ui->hugeIcon->setPixmap(KIcon("dialog-information").pixmap(128, 128));
    ui->errorText->setText(i18n("No printers have been configured or discovered"));
    ui->errorComment->hide();
    ui->addPrinterBtn->show();

    // Well, when there is no printer, there is nothing to add to a printer class
    // so we can actually hide the Add button nontheless?
    ui->addTB->hide();
    ui->removeTB->hide();
    ui->lineTB->hide();
    ui->printersTV->hide();
}

void PrintKCM::update()
{
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(server(KCupsServer)), this, SLOT(updateServer(KCupsServer)));
    request->getServerSettings();

    if (m_model->rowCount()) {        
        if (ui->stackedWidget->currentIndex() != 0) {
            ui->stackedWidget->setCurrentIndex(0);
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
        if (ui->printerDesc->destName() != destName) {
            ui->printerDesc->setPrinterIcon(index.data(Qt::DecorationRole).value<QIcon>());
            int type = index.data(PrinterModel::DestType).toUInt();
            // If we remove discovered printers, they will come
            // back to hunt us a bit later
            ui->removeTB->setEnabled(!(type & CUPS_PRINTER_DISCOVERED));
        }
        ui->printerDesc->setDestName(index.data(PrinterModel::DestName).toString(),
                                     index.data(PrinterModel::DestDescription).toString(),
                                     index.data(PrinterModel::DestIsClass).toBool());
        ui->printerDesc->setLocation(index.data(PrinterModel::DestLocation).toString());
        ui->printerDesc->setKind(index.data(PrinterModel::DestKind).toString());
        ui->printerDesc->setIsShared(index.data(PrinterModel::DestIsShared).toBool());
        ui->printerDesc->setIsDefault(index.data(PrinterModel::DestIsDefault).toBool());
        ui->printerDesc->setCommands(index.data(PrinterModel::DestCommands).toStringList());
        ui->printerDesc->setMarkers(index.data(PrinterModel::DestMarkers).value<QVariantHash>());

        ui->addTB->show();
        ui->removeTB->show();
        ui->lineTB->show();
        // Show the printer list only if there are more than 1 printer
        ui->printersTV->setVisible(m_model->rowCount() > 1);
    } else {
        // disable the printer action buttons if there is nothing to selected
        ui->removeTB->setEnabled(false);

        if (ui->stackedWidget->currentIndex() != 1) {
            // the model is empty and no problem happened
            noPrinters();
            ui->stackedWidget->setCurrentIndex(1);
        }
    }
}

void PrintKCM::on_addTB_clicked()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.AddPrinter"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.AddPrinter"),
                                             QLatin1String("AddPrinter"));
    message << static_cast<qulonglong>(winId());
    QDBusConnection::sessionBus().call(message);
}

void PrintKCM::addClass()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.AddPrinter"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.AddPrinter"),
                                             QLatin1String("AddClass"));
    message << static_cast<qulonglong>(winId());
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
        resp = KMessageBox::warningYesNo(this, msg, title);
        if (resp == KMessageBox::Yes) {
            KCupsRequest *request = new KCupsRequest;
            request->deletePrinter(index.data(PrinterModel::DestName).toString());
            request->waitTillFinished();
            request->deleteLater();
        }
    }
}

void PrintKCM::updateServer(const KCupsServer &server)
{
    m_showSharedPrinters->setChecked(server.showSharedPrinters());
    m_shareConnectedPrinters->setChecked(server.sharePrinters());
    m_allowPrintringFromInternet->setChecked(server.allowPrintingFromInternet());
    m_allowRemoteAdmin->setChecked(server.allowRemoteAdmin());
    m_allowUsersCancelAnyJob->setChecked(server.allowUserCancelAnyJobs());

    sender()->deleteLater();
}

void PrintKCM::updateServerFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    if (request->hasError()) {
        if (request->error() == IPP_SERVICE_UNAVAILABLE) {
            // Server is restarting, update the settings in one second
            QTimer::singleShot(1000, this, SLOT(update()));
        } else {
            qWarning() << "Failed to set server settings" << request->error() << request->errorMsg();
            KMessageBox::sorry(this, request->errorMsg(), request->serverError());

            // Force the settings to be retrieved again
            update();
        }
    }
    request->deleteLater();
}

void PrintKCM::systemPreferencesTriggered()
{
    KCupsServer server;
    server.setShowSharedPrinters(m_showSharedPrinters->isChecked());
    server.setSharePrinters(m_shareConnectedPrinters->isChecked());
    server.setAllowPrintingFromInternet(m_allowPrintringFromInternet->isChecked());
    server.setAllowRemoteAdmin(m_allowRemoteAdmin->isChecked());
    server.setAllowUserCancelAnyJobs(m_allowUsersCancelAnyJob->isChecked());
    KCupsRequest *request = server.commit();
    connect(request, SIGNAL(finished()), this, SLOT(updateServerFinished()));
}
