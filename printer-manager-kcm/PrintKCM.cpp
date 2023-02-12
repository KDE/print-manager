/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrintKCM.h"

#include "ui_PrintKCM.h"

#include <config.h>

#include <PrinterModel.h>
#include <PrinterSortFilterModel.h>
#include "PrinterDelegate.h"
#include "PrinterDescription.h"

#include <KMessageBox>
#include <KAboutData>
#include <KIO/CommandLauncherJob>

#include <QIcon>
#include <QMenu>
#include <KCupsRequest.h>
#include <NoSelectionRectDelegate.h>

#include <cups/cups.h>


K_PLUGIN_CLASS_WITH_JSON(PrintKCM, "kcm_printer_manager.json")

PrintKCM::PrintKCM(QWidget *parent, const QVariantList &args) :
    KCModule(parent, args),
    ui(new Ui::PrintKCM)
{
    auto aboutData = new KAboutData(QLatin1String("kcm_print"),
                                    i18n("Print settings"),
                                    QLatin1String(PM_VERSION),
                                    i18n("Print settings"),
                                    KAboutLicense::GPL,
                                    i18n("(C) 2010-2018 Daniel Nicoletti"));
    aboutData->addAuthor(QStringLiteral("Daniel Nicoletti"), QString(), QLatin1String("dantti12@gmail.com"));
    aboutData->addAuthor(QStringLiteral("Jan Grulich"), i18n("Port to Qt 5 / Plasma 5"), QStringLiteral("jgrulich@redhat.com"));
    setAboutData(aboutData);
    setButtons(NoAdditionalButton);

    ui->setupUi(this);

    connect(ui->printerDesc, &PrinterDescription::updateNeeded, this, &PrintKCM::update);

    // The printer list needs to increase in width according to the icon sizes
    // default dialog icon size is 32, this times 6 is 192 which is roughly the original width
    ui->printersTV->setMinimumWidth(192);

    auto addMenu = new QMenu(this);
    addMenu->addAction(i18nc("@action:intoolbar","Add a Printer Class"),
                       this, &PrintKCM::addClass);
    ui->addTB->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    ui->addTB->setToolTip(i18n("Add a new printer or a printer class"));
    ui->addTB->setMenu(addMenu);

    ui->removeTB->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    ui->removeTB->setToolTip(i18n("Remove Printer"));

    auto systemMenu = new QMenu(this);
    connect(systemMenu, &QMenu::triggered, this, &PrintKCM::systemPreferencesTriggered);
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
    connect(m_shareConnectedPrinters, &QAction::toggled, m_allowPrintringFromInternet, &QAction::setEnabled);
    connect(m_shareConnectedPrinters, &QAction::toggled, ui->printerDesc, &PrinterDescription::enableShareCheckBox);
    systemMenu->addSeparator();
    m_allowRemoteAdmin = systemMenu->addAction(i18nc("@action:intoolbar","Allow remote administration"));
    m_allowRemoteAdmin->setCheckable(true);
    m_allowUsersCancelAnyJob = systemMenu->addAction(i18nc("@action:intoolbar","Allow users to cancel any job (not just their own)"));
    m_allowUsersCancelAnyJob->setCheckable(true);

    ui->systemPreferencesTB->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    ui->systemPreferencesTB->setToolTip(i18n("Configure the global preferences"));
    ui->systemPreferencesTB->setMenu(systemMenu);

    m_model = new PrinterModel(this);
    m_sortModel = new PrinterSortFilterModel(this);
    m_sortModel->setSourceModel(m_model);
    ui->printersTV->setModel(m_sortModel);
    ui->printersTV->setItemDelegate(new NoSelectionRectDelegate(this));
    ui->printersTV->setItemDelegate(new PrinterDelegate(this));
    connect(ui->printersTV->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PrintKCM::update);
    connect(m_sortModel, &PrinterSortFilterModel::rowsInserted, this, &PrintKCM::update);
    connect(m_sortModel, &PrinterSortFilterModel::rowsRemoved, this, &PrintKCM::update);
    connect(m_model, &PrinterModel::dataChanged, this, &PrintKCM::update);
    connect(m_model, &PrinterModel::error, this, &PrintKCM::error);

    ui->addPrinterBtn->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    connect(ui->addPrinterBtn, &QPushButton::clicked, this, &PrintKCM::on_addTB_clicked);

    // Force the model update AFTER we setup the error signal
    m_model->update();

    // Make sure we update our server settings if the user change it on
    // another interface
    connect(KCupsConnection::global(), &KCupsConnection::serverAudit, this, &PrintKCM::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverStarted, this, &PrintKCM::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverStopped, this, &PrintKCM::getServerSettings);
    connect(KCupsConnection::global(), &KCupsConnection::serverRestarted, this, &PrintKCM::getServerSettings);

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
            showInfo(QIcon::fromTheme(QLatin1String("dialog-information")),
                     i18n("No printers have been configured or discovered"),
                     QString(),
                     true,
                     true);
        } else {
            showInfo(QIcon::fromTheme(QLatin1String("printer")),
                     QStringLiteral("<strong>%1</strong>").arg(errorTitle),
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

void PrintKCM::showInfo(const QIcon &icon, const QString &title, const QString &comment, bool showAddPrinter, bool showToolButtons)
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
    if (m_sortModel->rowCount()) {
        m_lastError = IPP_OK; // if the model has printers reset the error code
        if (ui->stackedWidget->currentIndex() != 0) {
            ui->stackedWidget->setCurrentIndex(0);
        }

        QItemSelection selection;
        // we need to map the selection to source to get the real indexes
        selection = ui->printersTV->selectionModel()->selection();
        // select the first printer if there are printers
        if (selection.indexes().isEmpty()) {
            ui->printersTV->selectionModel()->select(m_sortModel->index(0, 0), QItemSelectionModel::Select);
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
                                     index.data(PrinterModel::DestIsClass).toBool(),
                                     m_sortModel->rowCount() == 1);
        ui->printerDesc->setDestStatus(index.data(PrinterModel::DestStatus).toString());
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
        ui->printersTV->setVisible(m_sortModel->rowCount() > 1);
    } else {
        // disable the printer action buttons if there is nothing to selected
        ui->removeTB->setEnabled(false);

        if (m_lastError == IPP_OK) {
            // the model is empty and no problem happened
            showInfo(QIcon::fromTheme(QLatin1String("dialog-information")),
                     i18n("No printers have been configured or discovered"),
                     QString(),
                     true,
                     true);
        }
    }
}

void PrintKCM::on_addTB_clicked()
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("kde-add-printer"), { QStringLiteral("--add-printer") });
    job->start();
}

void PrintKCM::addClass()
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("kde-add-printer"), { QStringLiteral("--add-class") });
    job->start();
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
        auto systemMenu = qobject_cast<QMenu*>(sender());
        m_serverRequest = new KCupsRequest;
        m_serverRequest->setProperty("interactive", static_cast<bool>(systemMenu));
        connect(m_serverRequest, &KCupsRequest::finished, this, &PrintKCM::getServerSettingsFinished);
        m_serverRequest->getServerSettings();
    }
}

void PrintKCM::getServerSettingsFinished(KCupsRequest *request)
{
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
        if (request->property("interactive").toBool()) {
            KMessageBox::detailedError(this,
                                       i18nc("@info", "Failed to get server settings"),
                                       request->errorMsg(),
                                       i18nc("@title:window", "Failed"));
        }
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

    m_serverRequest = nullptr;
}

void PrintKCM::updateServerFinished(KCupsRequest *request)
{
    if (request->hasError()) {
        if (request->error() == IPP_SERVICE_UNAVAILABLE ||
                request->error() == IPP_INTERNAL_ERROR ||
                request->error() == IPP_AUTHENTICATION_CANCELED) {
            // Server is restarting, or auth was canceled, update the settings in one second
            QTimer::singleShot(1000, this, &PrintKCM::update);
        } else {
            KMessageBox::detailedError(this,
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
    auto request = new KCupsRequest;
    connect(request, &KCupsRequest::finished, this, &PrintKCM::updateServerFinished);
    request->setServerSettings(server);
}

#include "PrintKCM.moc"
