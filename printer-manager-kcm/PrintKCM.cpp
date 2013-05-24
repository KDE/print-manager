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

#include "PrintKCM.h"

#include "ui_PrintKCM.h"

#include <config.h>

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
#include <NoSelectionRectDelegate.h>

#include <cups/cups.h>

K_PLUGIN_FACTORY(PrintKCMFactory, registerPlugin<PrintKCM>();)
K_EXPORT_PLUGIN(PrintKCMFactory("kcm_print"))

PrintKCM::PrintKCM(QWidget *parent, const QVariantList &args) :
    KCModule(PrintKCMFactory::componentData(), parent, args),
    ui(new Ui::PrintKCM),
    m_lastError(-1), // Force the error to run on the first time
    m_serverRequest(0)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kcm_print",
                               "print-manager",
                               ki18n("Print settings"),
                               PM_VERSION,
                               ki18n("Print settings"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2010-2012 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(NoAdditionalButton);
    KGlobal::insertCatalog(QLatin1String("print-manager"));

    ui->setupUi(this);
    
    connect(ui->printerDesc, SIGNAL(updateNeeded()), SLOT(update()));

    QMenu *addMenu = new QMenu(this);
    addMenu->addAction(i18nc("@action:intoolbar","Add a Printer Class"),
                       this, SLOT(addClass()));
    ui->addTB->setIcon(KIcon("list-add"));
    ui->addTB->setToolTip(i18n("Add a new printer or a printer class"));
    ui->addTB->setMenu(addMenu);

    ui->removeTB->setIcon(KIcon("list-remove"));
    ui->removeTB->setToolTip(i18n("Remove Printer"));

    QMenu *systemMenu = new QMenu(this);
    connect(systemMenu, SIGNAL(aboutToShow()), this, SLOT(getServerSettings()));
    connect(systemMenu, SIGNAL(triggered(QAction*)), this, SLOT(systemPreferencesTriggered()));
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    m_showSharedPrinters = systemMenu->addAction(i18nc("@action:intoolbar","Show printers shared by other systems"));
    m_showSharedPrinters->setCheckable(true);
    systemMenu->addSeparator();
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    m_shareConnectedPrinters = systemMenu->addAction(i18nc("@action:intoolbar","Share printers connected to this system"));
    m_shareConnectedPrinters->setCheckable(true);
    m_allowPrintringFromInternet = systemMenu->addAction(i18nc("@action:intoolbar","Allow printing from the Internet"));
    m_allowPrintringFromInternet->setCheckable(true);
    m_allowPrintringFromInternet->setEnabled(false);
    connect(m_shareConnectedPrinters, SIGNAL(toggled(bool)), m_allowPrintringFromInternet, SLOT(setEnabled(bool)));
    connect(m_shareConnectedPrinters, SIGNAL(toggled(bool)), ui->printerDesc, SLOT(enableShareCheckBox(bool)));
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
    ui->printersTV->setItemDelegate(new NoSelectionRectDelegate(this));
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

    // Make sure we update our server settings if the user change it on
    // another interface
    connect(KCupsConnection::global(), SIGNAL(serverAudit(QString)), this, SLOT(getServerSettings()));
    connect(KCupsConnection::global(), SIGNAL(serverRestarted(QString)), this, SLOT(getServerSettings()));
    connect(KCupsConnection::global(), SIGNAL(serverStarted(QString)), this, SLOT(getServerSettings()));
    connect(KCupsConnection::global(), SIGNAL(serverStopped(QString)), this, SLOT(getServerSettings()));

    // We need to know the server settings so we disable the
    // share printer checkbox if sharing is disabled on the server
    getServerSettings();
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
            showInfo(KIcon("dialog-information"),
                     i18n("No printers have been configured or discovered"),
                     QString(),
                     true,
                     true);
        } else {
            showInfo(KIcon("printer", KIconLoader::global(), QStringList() << "" << "dialog-error"),
                     QString("<strong>%1</strong>").arg(errorTitle),
                     errorMsg,
                     false,
                     false);
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

void PrintKCM::showInfo(const KIcon &icon, const QString &title, const QString &comment, bool showAddPrinter, bool showToolButtons)
{
    ui->hugeIcon->setPixmap(icon.pixmap(128, 128));
    ui->errorText->setText(title);
    ui->errorComment->setVisible(!comment.isEmpty());
    ui->errorComment->setText(comment);
    ui->addPrinterBtn->setVisible(showAddPrinter);

    // Well, when there is no printer, there is nothing to add to a printer class
    // so we can actually hide the Add button nontheless?
    ui->addTB->setVisible(!showAddPrinter && showToolButtons);
    ui->removeTB->setVisible(!showAddPrinter && showToolButtons);
    ui->lineTB->setVisible(!showAddPrinter && showToolButtons);
    ui->printersTV->setVisible(!showAddPrinter && showToolButtons);

    // Make sure we are visible
    ui->stackedWidget->setCurrentIndex(1);
}

void PrintKCM::update()
{
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
        if (m_model->rowCount() == 1) {
          ui->printerDesc->setDestStatus(index.data(PrinterModel::DestStatus).toString());
        } else {
          ui->printerDesc->setDestStatus(QString());
        }
        ui->printerDesc->setLocation(index.data(PrinterModel::DestLocation).toString());
        ui->printerDesc->setKind(index.data(PrinterModel::DestKind).toString());
        ui->printerDesc->setIsShared(index.data(PrinterModel::DestIsShared).toBool());
        ui->printerDesc->setAcceptingJobs(index.data(PrinterModel::DestIsAcceptingJobs).toBool());
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

        if (m_lastError == IPP_OK) {
            // the model is empty and no problem happened
            showInfo(KIcon("dialog-information"),
                     i18n("No printers have been configured or discovered"),
                     QString(),
                     true,
                     true);
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
            QPointer<KCupsRequest> request = new KCupsRequest;
            request->deletePrinter(index.data(PrinterModel::DestName).toString());
            request->waitTillFinished();
            if (request) {
                request->deleteLater();
            }
        }
    }
}

void PrintKCM::getServerSettings()
{
    if (!m_serverRequest) {
        m_serverRequest = new KCupsRequest;
        connect(m_serverRequest, SIGNAL(finished()),
                this, SLOT(getServerSettingsFinished()));
        m_serverRequest->getServerSettings();
    }
}

void PrintKCM::getServerSettingsFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());

    // When we don't have any destinations error is set to IPP_NOT_FOUND
    // we can safely ignore the error since it DOES bring the server settings
    bool error = request->hasError() && request->error() != IPP_NOT_FOUND;

#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    m_showSharedPrinters->setEnabled(!error);
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    m_shareConnectedPrinters->setEnabled(!error);
    m_allowRemoteAdmin->setEnabled(!error);
    m_allowUsersCancelAnyJob->setEnabled(!error);

    if (error) {
        KMessageBox::detailedSorry(this,
                                   i18nc("@info", "Failed to get server settings"),
                                   request->errorMsg(),
                                   i18nc("@title:window", "Failed"));
    } else {
        KCupsServer server = request->serverSettings();

#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
        m_showSharedPrinters->setChecked(server.showSharedPrinters());
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
        m_shareConnectedPrinters->setChecked(server.sharePrinters());
        m_allowPrintringFromInternet->setChecked(server.allowPrintingFromInternet());
        m_allowRemoteAdmin->setChecked(server.allowRemoteAdmin());
        m_allowUsersCancelAnyJob->setChecked(server.allowUserCancelAnyJobs());
    }

    request->deleteLater();

    m_serverRequest = 0;
}

void PrintKCM::updateServerFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    if (request->hasError()) {
        if (request->error() == IPP_SERVICE_UNAVAILABLE ||
            request->error() == IPP_INTERNAL_ERROR ||
            request->error() == IPP_AUTHENTICATION_CANCELED) {
            // Server is restarting, or auth was canceled, update the settings in one second
            QTimer::singleShot(1000, this, SLOT(update()));
        } else {
            KMessageBox::detailedSorry(this,
                                       i18nc("@info", "Failed to configure server settings"),
                                       request->errorMsg(),
                                       request->serverError());

            // Force the settings to be retrieved again
            update();
        }
    }
    request->deleteLater();
}

void PrintKCM::systemPreferencesTriggered()
{
    KCupsServer server;
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    server.setShowSharedPrinters(m_showSharedPrinters->isChecked());
#endif // CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR < 6
    server.setSharePrinters(m_shareConnectedPrinters->isChecked());
    server.setAllowPrintingFromInternet(m_allowPrintringFromInternet->isChecked());
    server.setAllowRemoteAdmin(m_allowRemoteAdmin->isChecked());
    server.setAllowUserCancelAnyJobs(m_allowUsersCancelAnyJob->isChecked());
    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(finished()), this, SLOT(updateServerFinished()));
    request->setServerSettings(server);
}
