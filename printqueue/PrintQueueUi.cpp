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

#include "PrintQueueUi.h"
#include "ui_PrintQueueUi.h"

#include "PrintQueueModel.h"
#include "PrintQueueSortFilterProxyModel.h"

#include <KCupsRequest.h>
#include <KCupsPrinter.h>
#include <NoSelectionRectDelegate.h>

#include <QPainter>
#include <QToolBar>
#include <QMenu>
#include <QByteArray>
#include <QStringBuilder>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

#include <KMessageBox>
#include <KDebug>

#define PRINTER_ICON_SIZE 92

PrintQueueUi::PrintQueueUi(const KCupsPrinter &printer, QWidget *parent) :
    KDialog(parent),
    ui(new Ui::PrintQueueUi),
    m_destName(printer.name()),
    m_preparingMenu(false),
    m_printerPaused(false),
    m_lastState(0)
{
    ui->setupUi(mainWidget());

    // since setupUi needs to setup on the mainWidget()
    // we need to manually connect the buttons
    connect(ui->cancelJobPB, SIGNAL(clicked()), this, SLOT(cancelJob()));
    connect(ui->holdJobPB, SIGNAL(clicked()), this, SLOT(holdJob()));
    connect(ui->resumeJobPB, SIGNAL(clicked()), this, SLOT(resumeJob()));

    connect(ui->pausePrinterPB, SIGNAL(clicked()), this, SLOT(pausePrinter()));
    connect(ui->configurePrinterPB, SIGNAL(clicked()), this, SLOT(configurePrinter()));

    connect(ui->whichJobsCB, SIGNAL(currentIndexChanged(int)), this, SLOT(whichJobsIndexChanged(int)));

    // Needed so we have our dialog size saved
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowIcon(printer.icon());
    if (printer.info().isEmpty()) {
        m_title = printer.name();
    } else {
        m_title = printer.name() % QLatin1String(" - ") % printer.info();
    }
    setWindowTitle(m_title);
    setButtons(0);
    setSizeGripEnabled(true);
    (void) minimumSizeHint(); //Force the dialog to be laid out now
    layout()->setContentsMargins(0,0,0,0);

    m_isClass = printer.isClass();

    // setup default options
    ui->jobsView->setCornerWidget(new QWidget);

    setupButtons();

    // loads the standard key icon
    m_printerIcon = printer.icon().pixmap(PRINTER_ICON_SIZE, PRINTER_ICON_SIZE);
    ui->iconL->setPixmap(m_printerIcon);

    m_pauseIcon = KIconLoader::global()->loadIcon("media-playback-pause",
                                                  KIconLoader::NoGroup,
                                                  KIconLoader::SizeMedium,
                                                  KIconLoader::DefaultState,
                                                  QStringList(),
                                                  0,
                                                  true);

    ui->printerStatusMsgL->setText(QString());

    // setup the jobs model
    m_model = new PrintQueueModel(printer.name(), winId(), this);
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateButtons()));
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(update()));
    m_proxyModel = new PrintQueueSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setDynamicSortFilter(true);

    ui->jobsView->setModel(m_proxyModel);
    ui->jobsView->setItemDelegate(new NoSelectionRectDelegate(this));
    // sort by status column means the jobs will be sorted by the queue order
    ui->jobsView->sortByColumn(PrintQueueModel::ColStatus, Qt::AscendingOrder);
    connect(ui->jobsView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateButtons()));
    connect(ui->jobsView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    ui->jobsView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->jobsView->header(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showHeaderContextMenu(QPoint)));

    QHeaderView *header = ui->jobsView->header();
    header->setResizeMode(QHeaderView::Interactive);
    header->setStretchLastSection(false);
    header->setResizeMode(PrintQueueModel::ColStatus,        QHeaderView::ResizeToContents);
    header->setResizeMode(PrintQueueModel::ColName,          QHeaderView::Stretch);
    header->setResizeMode(PrintQueueModel::ColUser,          QHeaderView::ResizeToContents);
    header->setResizeMode(PrintQueueModel::ColCreated,       QHeaderView::ResizeToContents);
    header->setResizeMode(PrintQueueModel::ColCompleted,     QHeaderView::ResizeToContents);
    header->setResizeMode(PrintQueueModel::ColPages,         QHeaderView::ResizeToContents);
    header->setResizeMode(PrintQueueModel::ColProcessed,     QHeaderView::ResizeToContents);
    header->setResizeMode(PrintQueueModel::ColSize,          QHeaderView::ResizeToContents);
    header->setResizeMode(PrintQueueModel::ColStatusMessage, QHeaderView::ResizeToContents);
    header->setResizeMode(PrintQueueModel::ColPrinter,       QHeaderView::ResizeToContents);

    KConfig config("print-manager");
    KConfigGroup printQueue(&config, "PrintQueue");
    if (printQueue.hasKey("ColumnState")) {
        // restore the header state order
        header->restoreState(printQueue.readEntry("ColumnState", QByteArray()));
    } else {
        // Hide some columns ColPrinter
        header->hideSection(PrintQueueModel::ColPrinter);
        header->hideSection(PrintQueueModel::ColUser);
        header->hideSection(PrintQueueModel::ColCompleted);
        header->hideSection(PrintQueueModel::ColSize);
    }

    // This is emitted when a printer is modified
    connect(KCupsConnection::global(),
            SIGNAL(printerModified(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(updatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer has it's state changed
    connect(KCupsConnection::global(),
            SIGNAL(printerStateChanged(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(updatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is stopped
    connect(KCupsConnection::global(),
            SIGNAL(printerStopped(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(updatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is restarted
    connect(KCupsConnection::global(),
            SIGNAL(printerRestarted(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(updatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is shutdown
    connect(KCupsConnection::global(),
            SIGNAL(printerShutdown(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(updatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer is removed
    connect(KCupsConnection::global(),
            SIGNAL(printerDeleted(QString,QString,QString,uint,QString,bool)),
            this,
            SLOT(updatePrinter(QString,QString,QString,uint,QString,bool)));

    // This is emitted when a printer/queue is changed
    // Deprecated stuff that works better than the above
    connect(KCupsConnection::global(), SIGNAL(rhPrinterAdded(QString)),
            this, SLOT(updatePrinter(QString)));

    connect(KCupsConnection::global(), SIGNAL(rhPrinterRemoved(QString)),
            this, SLOT(updatePrinter(QString)));

    connect(KCupsConnection::global(), SIGNAL(rhQueueChanged(QString)),
            this, SLOT(updatePrinter(QString)));

    updatePrinter(m_destName);

    // Restore the dialog size
    restoreDialogSize(printQueue);
}

PrintQueueUi::~PrintQueueUi()
{
    KConfig config("print-manager");
    KConfigGroup configGroup(&config, "PrintQueue");
    // save the header state order
    configGroup.writeEntry("ColumnState", ui->jobsView->header()->saveState());

    // Save the dialog size
    saveDialogSize(configGroup);
}

int PrintQueueUi::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return PrintQueueModel::LastColumn;
    }
    return 0;
}

void PrintQueueUi::setState(int state, const QString &message)
{
    kDebug() << state << message;
    if (state != m_lastState ||
        ui->printerStatusMsgL->text() != message) {
        // save the last state so the ui doesn't need to keep updating
        if (ui->printerStatusMsgL->text() != message) {
            ui->printerStatusMsgL->setText(message);
        }
        m_lastState = state;

        QPixmap icon(m_printerIcon);
        m_printerPaused = false;
        switch (state) {
        case KCupsPrinter::Idle:
            ui->statusL->setText(i18n("Printer ready"));
            ui->pausePrinterPB->setText(i18n("Pause Printer"));
            ui->pausePrinterPB->setIcon(KIcon("media-playback-pause"));
            break;
        case KCupsPrinter::Printing:
            if (!m_title.isNull()) {
                QString jobTitle = m_model->processingJob();
                if (jobTitle.isEmpty()) {
                    ui->statusL->setText(i18n("Printing..."));
                } else {
                    ui->statusL->setText(i18n("Printing '%1'", jobTitle));
                }
                ui->pausePrinterPB->setText(i18n("Pause Printer"));
                ui->pausePrinterPB->setIcon(KIcon("media-playback-pause"));
            }
            break;
        case KCupsPrinter::Stoped:
            m_printerPaused = true;
            ui->statusL->setText(i18n("Printer paused"));
            ui->pausePrinterPB->setText(i18n("Resume Printer"));
            ui->pausePrinterPB->setIcon(KIcon("media-playback-start"));
            // create a paiter to paint the action icon over the key icon
            {
                QPainter painter(&icon);
                // the emblem icon to size 32
                int overlaySize = KIconLoader::SizeMedium;
                QPoint startPoint;
                // bottom right corner
                startPoint = QPoint(PRINTER_ICON_SIZE - overlaySize - 2,
                                    PRINTER_ICON_SIZE - overlaySize - 2);
                painter.drawPixmap(startPoint, m_pauseIcon);
            }
            break;
        default :
            ui->statusL->setText(i18n("Printer state unknown"));
            break;
        }
        // set the printer icon
        setWindowIcon(icon);
    }
}

void PrintQueueUi::showContextMenu(const QPoint &point)
{
    // check if the click was actually over a job
    if (!ui->jobsView->indexAt(point).isValid() || m_preparingMenu) {
        return;
    }
    m_preparingMenu = true;

    bool moveTo = false;
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = m_proxyModel->mapSelectionToSource(ui->jobsView->selectionModel()->selection());
    // if the selection is empty the user clicked on an empty space
    if (!selection.indexes().isEmpty()) {
        foreach (const QModelIndex &index, selection.indexes()) {
            if (index.column() == 0 && index.flags() & Qt::ItemIsDragEnabled) {
                // Found a move to item
                moveTo = true;
                break;
            }
        }
        // if we can move a job create the menu
        if (moveTo) {
            // context menu
            QMenu *menu = new QMenu(this);
            // move to menu
            QMenu *moveToMenu = new QMenu(i18n("Move to"), this);

            // get printers we can move to
            QPointer<KCupsRequest> request = new KCupsRequest;
            QStringList attr;
            attr << KCUPS_PRINTER_NAME;
            attr << KCUPS_PRINTER_INFO;
            request->getPrinters(attr);
            request->waitTillFinished();
            if (!request) {
                return;
            }
            KCupsPrinters printers = request->printers();
            request->deleteLater();

            foreach (const KCupsPrinter &printer, printers) {
                // If there is a printer and it's not the current one add it
                // as a new destination
                if (printer.name() != m_destName) {
                    QAction *action = moveToMenu->addAction(printer.info());
                    action->setData(printer.name());
                }
            }

            if (!moveToMenu->isEmpty()) {
                menu->addMenu(moveToMenu);
                // show the menu on the right point
                QAction *action = menu->exec(ui->jobsView->mapToGlobal(point));
                if (action) {
                    // move the job
                    modifyJob(PrintQueueModel::Move, action->data().toString());
                }
            }
        }
    }
    m_preparingMenu = false;
}

void PrintQueueUi::showHeaderContextMenu(const QPoint &point)
{
    // Displays a menu containing the header name, and
    // a check box to indicate if it's being shown
    QMenu *menu = new QMenu(this);
    for (int i = 0; i < m_proxyModel->columnCount(); i++) {
        QAction *action;
        QString name;
        name = m_proxyModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
        action = menu->addAction(name);
        action->setCheckable(true);
        action->setChecked(!ui->jobsView->header()->isSectionHidden(i));
        action->setData(i);
    }

    QAction *action = menu->exec(ui->jobsView->header()->mapToGlobal(point));
    if (action) {
        int section = action->data().toInt();
        if (action->isChecked()) {
            ui->jobsView->header()->showSection(section);
        } else {
            ui->jobsView->header()->hideSection(section);
        }
    }
}

void PrintQueueUi::updatePrinter(const QString &printer)
{
    kDebug() << printer << m_destName;
    if (printer != m_destName) {
        // It was another printer that changed
        return;
    }

    QStringList attr;
    attr << KCUPS_PRINTER_INFO;
    attr << KCUPS_PRINTER_TYPE;
    attr << KCUPS_PRINTER_STATE;
    attr << KCUPS_PRINTER_STATE_MESSAGE;

    KCupsRequest *request = new KCupsRequest;
    connect(request, SIGNAL(finished()), this, SLOT(getAttributesFinished()));
    request->getPrinterAttributes(printer, m_isClass, attr);
}

void PrintQueueUi::updatePrinter(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs)
{
    // REALLY? all these parameters just to say foo was added??
    Q_UNUSED(text)
    Q_UNUSED(printerUri)
    Q_UNUSED(printerState)
    Q_UNUSED(printerStateReasons)
    Q_UNUSED(printerIsAcceptingJobs)
    kDebug() << printerName << printerStateReasons;

    updatePrinter(printerName);
}

void PrintQueueUi::getAttributesFinished()
{
    KCupsRequest *request = qobject_cast<KCupsRequest *>(sender());
    kDebug() << request->hasError() << request->printers().isEmpty();

    if (request->hasError() || request->printers().isEmpty()) {
        // if cups stops we disable our queue
        setEnabled(false);
        request->deleteLater(); // DO not delete before using as the request is in another thread
        return;
    } else if (isEnabled() == false) {
        // if cups starts again we enable our queue
        setEnabled(true);
    }

    KCupsPrinter printer = request->printers().first();

    // get printer-info
    if (printer.info().isEmpty()) {
        m_title = printer.name();
    } else {
        m_title = printer.name() % QLatin1String(" - ") % printer.info();
    }

    // get printer-state
    setState(printer.state(), printer.stateMsg());

    // store if the printer is a class
    m_isClass = printer.isClass();

    request->deleteLater();

    update();
}

void PrintQueueUi::update()
{
    // Set window title
    if (m_model->rowCount()) {
        if (m_destName.isNull()) {
            setWindowTitle(i18np("All Printers (%1 Job)", "All Printers (%1 Jobs)", m_model->rowCount()));
        } else {
            setWindowTitle(i18np("%2 (%1 Job)", "%2 (%1 Jobs)", m_model->rowCount(), m_title));
        }
    } else {
        setWindowTitle(m_destName.isNull() ? i18n("All Printers") : m_title);
    }
}

void PrintQueueUi::updateButtons()
{
    bool cancel, hold, release;
    // Set all options to false
    cancel = hold = release = false;

    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = m_proxyModel->mapSelectionToSource(ui->jobsView->selectionModel()->selection());
    // enable or disable the job action buttons if something is selected
    if (!selection.indexes().isEmpty()) {
        foreach (const QModelIndex &index, selection.indexes()) {
            if (index.column() == 0) {
                switch (static_cast<ipp_jstate_t>(index.data(PrintQueueModel::JobState).toInt()))
                {
                    case IPP_JOB_CANCELED :
                    case IPP_JOB_COMPLETED :
                    case IPP_JOB_ABORTED :
                        break;
                    case IPP_JOB_HELD :
                    case IPP_JOB_STOPPED :
                        release = true;
                        cancel = true;
                        break;
                    default:
                        cancel = hold = true;
                        break;
                }
            }
        }
    }

    ui->cancelJobPB->setEnabled(cancel);
    ui->holdJobPB->setEnabled(hold);
    ui->resumeJobPB->setEnabled(release);
}

void PrintQueueUi::modifyJob(int action, const QString &destName)
{
    // get all selected indexes
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = m_proxyModel->mapSelectionToSource(ui->jobsView->selectionModel()->selection());
    foreach (const QModelIndex &index, selection.indexes()) {
        if (index.column() == 0) {
            KCupsRequest *request;
            request = m_model->modifyJob(index.row(),
                                        static_cast<PrintQueueModel::JobAction>(action),
                                        destName);
            if (!request) {
                // probably the job already has this state
                // or this is an unknown action
                continue;
            }
            request->waitTillFinished();
            if (request->hasError()) {
                QString msg, jobName;
                jobName = m_model->item(index.row(), static_cast<int>(PrintQueueModel::ColName))->text();
                switch (action) {
                case PrintQueueModel::Cancel:
                    msg = i18n("Failed to cancel '%1'", jobName);
                    break;
                case PrintQueueModel::Hold:
                    msg = i18n("Failed to hold '%1'", jobName);
                    break;
                case PrintQueueModel::Release:
                    msg = i18n("Failed to release '%1'", jobName);
                    break;
                case PrintQueueModel::Move:
                    msg = i18n("Failed to move '%1' to '%2'", jobName, destName);
                    break;
                }
                KMessageBox::detailedSorry(this,
                                           msg,
                                           request->errorMsg(),
                                           i18n("Failed"));
            }
            request->deleteLater();
        }
    }
}

void PrintQueueUi::pausePrinter()
{
    // STOP and RESUME printer
    QPointer<KCupsRequest> request = new KCupsRequest;
    if (m_printerPaused) {
        kDebug() << m_destName << "m_printerPaused";
        request->resumePrinter(m_destName);
    } else {
        kDebug() << m_destName << "NOT m_printerPaused";
        request->pausePrinter(m_destName);
    }
    request->waitTillFinished();
    if (request) {
        request->deleteLater();
    }
}

void PrintQueueUi::configurePrinter()
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.ConfigurePrinter"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.ConfigurePrinter"),
                                             QLatin1String("ConfigurePrinter"));
    message << qVariantFromValue(m_destName);
    QDBusConnection::sessionBus().send(message);
}

void PrintQueueUi::cancelJob()
{
    // CANCEL a job
    modifyJob(PrintQueueModel::Cancel);
}

void PrintQueueUi::holdJob()
{
    // HOLD a job
    modifyJob(PrintQueueModel::Hold);
}

void PrintQueueUi::resumeJob()
{
    // RESUME a job
    modifyJob(PrintQueueModel::Release);
}

void PrintQueueUi::whichJobsIndexChanged(int index)
{
    int whichJobs;
    switch (index) {
    case 1:
        whichJobs = CUPS_WHICHJOBS_COMPLETED;
        break;
    case 2:
        whichJobs = CUPS_WHICHJOBS_ALL;
        break;
    default:
        whichJobs = CUPS_WHICHJOBS_ACTIVE;
    }
    m_model->setWhichJobs(whichJobs);
}

void PrintQueueUi::setupButtons()
{
    // setup jobs buttons

    // cancel action
    ui->cancelJobPB->setIcon(KIcon("dialog-cancel"));

    // hold job action
    ui->holdJobPB->setIcon(KIcon("document-open-recent"));

    // resume job action
    // TODO we need a new icon
    ui->resumeJobPB->setIcon(KIcon("media-playback-play"));

    ui->whichJobsCB->setItemIcon(0, KIcon("view-filter"));
    ui->whichJobsCB->setItemIcon(1, KIcon("view-filter"));
    ui->whichJobsCB->setItemIcon(2, KIcon("view-filter"));

    // stop start printer
    ui->pausePrinterPB->setIcon(KIcon("media-playback-pause"));

    // configure printer
    ui->configurePrinterPB->setIcon(KIcon("configure"));
}

void PrintQueueUi::closeEvent(QCloseEvent *event)
{
    // emits finished signal to be removed the cache
    emit finished();
    QWidget::closeEvent(event);
}

#include "PrintQueueUi.moc"
