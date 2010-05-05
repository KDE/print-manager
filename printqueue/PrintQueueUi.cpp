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

#include "PrintQueueUi.h"

#include "PrintQueueModel.h"
#include "PrintQueueSortFilterProxyModel.h"

#include <ConfigureDialog.h>
#include <QCups.h>

#include <QPainter>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QByteArray>

#include <KMessageBox>
#include <KDebug>

#define PRINTER_ICON_SIZE 92

PrintQueueUi::PrintQueueUi(const QString &destName, bool isClass, QWidget *parent)
 : QWidget(parent),
   m_destName(destName),
   m_isClass(isClass),
   m_preparingMenu(false),
   m_lastState(NULL),
   m_cfgDlg(0)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // setup default options
//     setWindowIcon(KIcon("printer").pixmap(32));
    setWindowTitle(m_destName.isNull() ? i18n("All printers") : m_destName);
    jobsView->setCornerWidget(new QWidget);

    setupButtons();

    // loads the standard key icon
    m_printerIcon = KIconLoader::global()->loadIcon("printer",
                                                    KIconLoader::NoGroup,
                                                    PRINTER_ICON_SIZE, // a not so huge icon
                                                    KIconLoader::DefaultState);
    iconL->setPixmap(m_printerIcon);

    m_pauseIcon = KIconLoader::global()->loadIcon("media-playback-pause",
                                                  KIconLoader::NoGroup,
                                                  KIconLoader::SizeMedium,
                                                  KIconLoader::DefaultState,
                                                  QStringList(),
                                                  0,
                                                  true);

    printerStatusMsgL->setText(QString());

    // setup the jobs model
    m_model = new PrintQueueModel(destName, winId(), this);
    connect(m_model, SIGNAL(dataChanged( const QModelIndex &, const QModelIndex &)),
            this, SLOT(updateButtons()));
    m_proxyModel = new PrintQueueSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setDynamicSortFilter(true);

    jobsView->setModel(m_proxyModel);
    // sort by status column means the jobs will be sorted by the queue order
    jobsView->sortByColumn(PrintQueueModel::ColStatus, Qt::AscendingOrder);
    connect(jobsView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(updateButtons()));
    connect(jobsView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
    jobsView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(jobsView->header(), SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showHeaderContextMenu(const QPoint &)));

    KConfig config("print-manager");
    KConfigGroup printQueue(&config, "PrintQueue");
    if (printQueue.hasKey("ColumnState")) {
        // restore the header state order
        jobsView->header()->restoreState(printQueue.readEntry("ColumnState", QByteArray()));
    } else {
        // Hide the sections after ColPrinter
        for (int i = PrintQueueModel::ColPrinter + 1; i < PrintQueueModel::LastColumn; i++) {
            jobsView->header()->hideSection(i);
        }
    }

    update();
}

PrintQueueUi::~PrintQueueUi()
{
    KConfig config("print-manager");
    KConfigGroup printQueue(&config, "PrintQueue");
    // save the header state order
    printQueue.writeEntry("ColumnState", jobsView->header()->saveState());
}

void PrintQueueUi::setState(int state, const QString &message)
{
    if (state != m_lastState ||
        printerStatusMsgL->text() != message) {
        // save the last state so the ui doesn't need to keep updating
        if (printerStatusMsgL->text() != message) {
            printerStatusMsgL->setText(message);
        }
        m_lastState = state;

        QPixmap icon(m_printerIcon);
        m_printerPaused = false;
        switch (state) {
        case DEST_IDLE :
            statusL->setText(i18n("Printer ready"));
            pausePrinterPB->setText(i18n("Pause Printer"));
            pausePrinterPB->setIcon(KIcon("media-playback-pause"));
            break;
        case DEST_PRINTING :
            if (!m_title.isNull()) {
                QString jobTitle = m_model->processingJob();
                if (jobTitle.isEmpty()) {
                    statusL->setText(i18n("Printing..."));
                } else {
                    statusL->setText(i18n("Printing '%1'", jobTitle));
                }
                pausePrinterPB->setText(i18n("Pause Printer"));
                pausePrinterPB->setIcon(KIcon("media-playback-pause"));
            }
            break;
        case DEST_STOPED :
            m_printerPaused = true;
            statusL->setText(i18n("Printer paused"));
            pausePrinterPB->setText(i18n("Resume Printer"));
            pausePrinterPB->setIcon(KIcon("media-playback-start"));
            // create a paiter to paint the action icon over the key icon
            {
                QPainter painter(&icon);
                // the the emblem icon to size 32
                int overlaySize = KIconLoader::SizeMedium;
                QPoint startPoint;
                // bottom right corner
                startPoint = QPoint(PRINTER_ICON_SIZE - overlaySize - 2,
                                    PRINTER_ICON_SIZE - overlaySize - 2);
                painter.drawPixmap(startPoint, m_pauseIcon);
            }
            break;
        default :
            statusL->setText(i18n("Printer state unknown"));
            break;
        }
        // set the printer icon
        setWindowIcon(icon);
    }
}

void PrintQueueUi::showContextMenu(const QPoint &point)
{
    // check if the click was actually over a job
    if (!jobsView->indexAt(point).isValid() || m_preparingMenu) {
        return;
    }
    m_preparingMenu = true;

    bool moveTo = false;
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = m_proxyModel->mapSelectionToSource(jobsView->selectionModel()->selection());
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
            QCups::Result *ret = QCups::getDests(-1, QStringList() << "printer-name" << "printer-info");
            ret->waitTillFinished();
            QCups::ReturnArguments dests = ret->result();
            ret->deleteLater();

            foreach (const QCups::Arguments &dest, dests) {
                // If there is a printer and it's not the current one add it
                // as a new destination
                if (dest["printer-name"].toString() != m_destName) {
                    QAction *action = moveToMenu->addAction(dest["printer-info"].toString());
                    action->setData(dest["printer-name"].toString());
                }
            }

            if (!moveToMenu->isEmpty()) {
                menu->addMenu(moveToMenu);
                // show the menu on the right point
                QAction *action = menu->exec(jobsView->mapToGlobal(point));
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
        action->setChecked(!jobsView->header()->isSectionHidden(i));
        action->setData(i);
    }

    QAction *action = menu->exec(jobsView->header()->mapToGlobal(point));
    if (action) {
        int section = action->data().toInt();
        if (action->isChecked()) {
            jobsView->header()->showSection(section);
        } else {
            jobsView->header()->hideSection(section);
        }
    }
}

void PrintQueueUi::getAttributesFinished()
{
    QCups::Result *ret = qobject_cast<QCups::Result*>(sender());
    QHash<QString, QVariant> attributes;
    if (!ret->result().isEmpty()){
        attributes = ret->result().first();
    }

    if (attributes.isEmpty()) {
        // if cups stops we disable our queue
        setEnabled(false);
        return;
    } else if (isEnabled() == false) {
        // if cups starts agina we enable our queue
        setEnabled(true);
    }

    // get printer-info
    if (attributes["printer-info"].toString().isEmpty()) {
        m_title = m_destName;
    } else {
        m_title = attributes["printer-info"].toString();
    }

    // get printer-state
    setState(attributes["printer-state"].toInt(),
             attributes["printer-state-message"].toString());

    // store if the printer is a class
    m_isClass = attributes["printer-type"].toInt() & CUPS_PRINTER_CLASS;

    m_model->updateModel();

    // Set window title
    if (m_model->rowCount()) {
        if (m_title.isNull()) {
            emit windowTitleChanged(i18np("All Printers (%1 Job)", "All Printers (%1 Jobs)", m_model->rowCount()));
        } else {
            emit windowTitleChanged(i18np("%2 (%1 Job)", "%2 (%1 Jobs)", m_model->rowCount(), m_title));
        }
    } else {
        emit windowTitleChanged(m_title.isNull() ? i18n("All Printers") : m_title);
    }

    ret->deleteLater();
}

void PrintQueueUi::update()
{
    QStringList attr;
    attr << "printer-info"
         << "printer-type"
         << "printer-state"
         << "printer-state-message";

    QHash<QString, QVariant> attributes;
    QCups::Result *ret = QCups::Dest::getAttributes(m_destName, m_isClass, attr);
    connect(ret, SIGNAL(finished()), this, SLOT(getAttributesFinished()));
}

void PrintQueueUi::updateButtons()
{
    bool cancel, hold, release;
    // Set all options to false
    cancel = hold = release = false;

    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = m_proxyModel->mapSelectionToSource(jobsView->selectionModel()->selection());
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

    cancelJobPB->setEnabled(cancel);
    holdJobPB->setEnabled(hold);
    resumeJobPB->setEnabled(release);
}

void PrintQueueUi::modifyJob(int action, const QString &destName)
{
    // get all selected indexes
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = m_proxyModel->mapSelectionToSource(jobsView->selectionModel()->selection());
    foreach (const QModelIndex &index, selection.indexes()) {
        if (index.column() == 0) {
            QCups::Result *result;
            result = m_model->modifyJob(index.row(),
                                        static_cast<PrintQueueModel::JobAction>(action),
                                        destName);
            if (!result) {
                // probably the job already has this state
                // or this is an unknown action
                continue;
            }
            result->waitTillFinished();
            if (result->hasError()) {
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
                                           result->lastErrorString(),
                                           i18n("Failed"));
            }
            result->deleteLater();
        }
    }
}

void PrintQueueUi::on_pausePrinterPB_clicked()
{
    // STOP and RESUME printer
    QCups::Result *ret;
    if (m_printerPaused) {
        ret = QCups::resumePrinter(m_destName);
    } else {
        ret = QCups::pausePrinter(m_destName);
    }
    ret->waitTillFinished();
    ret->deleteLater();
}

void PrintQueueUi::on_configurePrinterPB_clicked()
{
    if (m_cfgDlg) {
        return;
    }
    m_cfgDlg = new QCups::ConfigureDialog(m_destName, m_isClass, this);
    m_cfgDlg->exec();
    m_cfgDlg = 0;
}

void PrintQueueUi::on_cancelJobPB_clicked()
{
    // CANCEL a job
    modifyJob(PrintQueueModel::Cancel);
}

void PrintQueueUi::on_holdJobPB_clicked()
{
    // HOLD a job
    modifyJob(PrintQueueModel::Hold);
}

void PrintQueueUi::on_resumeJobPB_clicked()
{
    // RESUME a job
    modifyJob(PrintQueueModel::Release);
}

void PrintQueueUi::on_whichJobsCB_currentIndexChanged(int index)
{
    int whichJobs = CUPS_WHICHJOBS_ACTIVE;
    switch (index) {
    case 0:
        whichJobs = CUPS_WHICHJOBS_ACTIVE;
        break;
    case 1:
        whichJobs = CUPS_WHICHJOBS_COMPLETED;
        break;
    case 2:
        whichJobs = CUPS_WHICHJOBS_ALL;
        break;
    }
    m_model->setWhichJobs(whichJobs);
}

void PrintQueueUi::setupButtons()
{
    // setup jobs buttons

    // cancel action
    cancelJobPB->setIcon(KIcon("dialog-cancel"));

    // hold job action
    holdJobPB->setIcon(KIcon("document-open-recent"));

    // resume job action
    // TODO we need a new icon
    resumeJobPB->setIcon(KIcon("media-playback-play"));

    whichJobsCB->setItemIcon(0, KIcon("view-filter"));
    whichJobsCB->setItemIcon(1, KIcon("view-filter"));
    whichJobsCB->setItemIcon(2, KIcon("view-filter"));

    // stop start printer
    pausePrinterPB->setIcon(KIcon("media-playback-pause"));

    // configure printer
    configurePrinterPB->setIcon(KIcon("configure"));
}

void PrintQueueUi::closeEvent(QCloseEvent *event)
{
    // emits finished signal to be removed the cache
    emit finished();
    QWidget::closeEvent(event);
}

#include "PrintQueueUi.moc"
