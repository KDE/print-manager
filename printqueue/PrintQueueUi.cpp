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

#include <QCups.h>
#include <cups/cups.h>

#include <QSortFilterProxyModel>
#include <QPainter>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>

#include <KMessageBox>
#include <KDebug>

#define DEST_IDLE     '3'
#define DEST_PRINTING '4'
#define DEST_STOPED   '5'

#define PRINTER_ICON_SIZE 96

PrintQueueUi::PrintQueueUi(const QString &destName, QWidget *parent)
 : QWidget(parent),
   m_destName(destName),
   m_lastState(NULL)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // setup default options
    setWindowIcon(KIcon("printer").pixmap(32));
    iconL->setPixmap(KIcon("printer").pixmap(128));
    setWindowTitle(m_destName.isNull() ? i18n("All printers") : m_destName);

    setActions();

    // loads the standard key icon
    m_printerIcon = KIconLoader::global()->loadIcon("printer",
                                                    KIconLoader::NoGroup,
                                                    PRINTER_ICON_SIZE, // a not so huge icon
                                                    KIconLoader::DefaultState);

    m_pauseIcon = KIconLoader::global()->loadIcon("media-playback-pause",
                                                  KIconLoader::NoGroup,
                                                  KIconLoader::SizeMedium,
                                                  KIconLoader::DefaultState,
                                                  QStringList(),
                                                  0,
                                                  true);

    // setup the jobs model
    m_model = new PrintQueueModel(destName, winId(), this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setDynamicSortFilter(true);
    jobsView->setModel(m_proxyModel);
    connect(jobsView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(selectedJobs()));
    connect(jobsView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));

    update();
}

PrintQueueUi::~PrintQueueUi()
{
}

void PrintQueueUi::setState(const char &state)
{
    if (state != m_lastState) {
        QPixmap icon(m_printerIcon);
        m_printerPaused = false;
        switch (state) {
        case DEST_IDLE :
            statusL->setText(i18n("Printer ready"));
            actionStopStartPrinter->setText(i18n("Pause printer"));
            actionStopStartPrinter->setIcon(KIcon("media-playback-pause"));
            break;
        case DEST_PRINTING :
            if (!m_title.isNull()) {
                int num_jobs;
                cups_job_t *jobs;
                num_jobs = cupsGetJobs(&jobs, m_destName.toLocal8Bit().data(), 0, CUPS_WHICHJOBS_ACTIVE);

                QString jobTitle;
                for (int i = 0; i < num_jobs; i++) {
                    if (jobs[i].state == IPP_JOB_PROCESSING) {
                        jobTitle = QString::fromLocal8Bit(jobs[i].title);
                        break;
                    }
                }
                if (jobTitle.isEmpty()) {
                    statusL->setText(i18n("Printing..."));
                } else {
                    statusL->setText(i18n("Printing '%1'", jobTitle));
                }
                actionStopStartPrinter->setText(i18n("Pause printer"));
                actionStopStartPrinter->setIcon(KIcon("media-playback-pause"));
            }
            break;
        case DEST_STOPED :
            m_printerPaused = true;
            statusL->setText(i18n("Printer paused"));
            actionStopStartPrinter->setText(i18n("Resume printer"));
            actionStopStartPrinter->setIcon(KIcon("media-playback-start"));
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
        iconL->setPixmap(icon);
    }
}

void PrintQueueUi::showContextMenu(const QPoint &point)
{
    // check if the click was actually over a job
    if (!jobsView->indexAt(point).isValid()) {
        return;
    }

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
            cups_dest_t *dests;
            int num_dests = cupsGetDests(&dests);
            cups_dest_t *dest;
            int i;
            const char *value;

            for (i = num_dests, dest = dests; i > 0; i --, dest ++) {
                // If there is a printer and it's not the current one add it
                // as a new destination
                QString destName = QString::fromLocal8Bit(dest->name);
                if (dest->instance == NULL && m_destName != destName) {
                    value = cupsGetOption("printer-info", dest->num_options, dest->options);
                    QAction *action = moveToMenu->addAction(QString::fromLocal8Bit(value));
                    action->setData(destName);
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
            // don't leak
            cupsFreeDests(num_dests, dests);
        }
    }
}

void PrintQueueUi::update()
{

    cups_dest_t *dests;
    const char *value;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(m_destName.toLocal8Bit(), NULL, num_dests, dests);

    if (dest == NULL) {
        setEnabled(false);
        return;
    }
    // get printer-info
    value = cupsGetOption("printer-info", dest->num_options, dest->options);
    if (value) {
        m_title = QString::fromLocal8Bit(value);
    } else {
        m_title = m_destName;
    }

    // get printer-state
    value = cupsGetOption("printer-state", dest->num_options, dest->options);
    if (value) {
        setState(value[0]);
    }

    value = cupsGetOption("printer-state-reasons", dest->num_options, dest->options);
//     printf("%s (%s)\n", dest->name, value ? value : "no description");


    cupsFreeDests(num_dests, dests);

    m_model->updateModel();

    // Set window title
    if (m_model->rowCount()) {
        if (m_title.isNull()) {
            setWindowTitle(i18np("All printers (%1 job)", "All printers (%1 jobs)", m_model->rowCount()));
        } else {
            setWindowTitle(i18np("%2 (%1 job)", "%2 (%1 jobs)", m_model->rowCount(), m_title));
        }
    } else {
        setWindowTitle(m_title.isNull() ? i18n("All printers") : m_title);
    }
}

void PrintQueueUi::selectedJobs()
{
//     kDebug();
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
                switch ((ipp_jstate_t) index.data(PrintQueueModel::JobState).toInt())
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

    actionCancel->setEnabled(cancel);
    actionHold->setEnabled(hold);
    actionResume->setEnabled(release);
}

void PrintQueueUi::modifyJob(int action, const QString &destName)
{
    // get all selected indexes
    QItemSelection selection;
    // we need to map the selection to source to get the real indexes
    selection = m_proxyModel->mapSelectionToSource(jobsView->selectionModel()->selection());
    foreach (const QModelIndex &index, selection.indexes()) {
        if (index.column() == 0) {
            if (!m_model->modifyJob(index.row(), (PrintQueueModel::JobAction) action, destName)) {
                QString msg, jobName;
                jobName = m_model->item(index.row(), (int) PrintQueueModel::ColName)->text();
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
                    msg = i18n("Failed to release '%1' to '%2'", jobName, destName);
                    break;
                }
                KMessageBox::detailedSorry(this,
                                           msg,
                                           cupsLastErrorString(),
                                           i18n("Failed"));
                return;
            }
        }
    }
}

void PrintQueueUi::actionTriggered(QAction *action)
{
    if (action == actionCancel) {
        // CANCEL a job
        modifyJob(PrintQueueModel::Cancel);
    } else if (action == actionHold) {
        // HOLD a job
        modifyJob(PrintQueueModel::Hold);
    } else if (action == actionResume) {
        // HOLD a job
        modifyJob(PrintQueueModel::Release);
    } else if (action == actionStopStartPrinter) {
        // STOP and RESUME printer
        if (m_printerPaused) {
            QCups::resumePrinter(m_destName.toLocal8Bit());
        } else {
            QCups::pausePrinter(m_destName.toLocal8Bit());
        }
    } else if (action == actionActiveJobs ||
               action == actionCompletedJobs ||
               action == actionAllJobs) {
        // job filter
        m_filterJobs->setText(action->text());
        m_model->setWhichJobs(action->data().toInt());
    }
}

void PrintQueueUi::setActions()
{
    // setup actions
    // Left tool bar
    QToolBar *toolBar = new QToolBar(this);
    connect(toolBar, SIGNAL(actionTriggered(QAction *)), this, SLOT(actionTriggered(QAction *)));
    toolBarLayout->addWidget(toolBar);

    // cancel action
    actionCancel->setIcon(KIcon("dialog-cancel"));
    actionCancel->setEnabled(false);
    toolBar->addAction(actionCancel);

    // hold job action
    actionHold->setIcon(KIcon("document-open-recent"));
    actionHold->setEnabled(false);
    toolBar->addAction(actionHold);

    // resume job action
    actionResume->setIcon(KIcon("media-playback-play"));
    actionResume->setEnabled(false);
    toolBar->addAction(actionResume);

    toolBar->addSeparator();

    // stop start printer
    actionStopStartPrinter->setIcon(KIcon("media-playback-pause"));
    toolBar->addAction(actionStopStartPrinter);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Adds a spacer between tool bars
    toolBarLayout->addStretch();

    // RIGHT second tool bar for view and configure
    QToolBar *toolBar2 = new QToolBar(this);
    toolBar2->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBarLayout->addWidget(toolBar2);

    // filter menu
    QMenu *menu = new QMenu(this);
    // add jobs filter button
    m_filterJobs = new QToolButton(this);
    m_filterJobs->setMenu(menu);
    m_filterJobs->setPopupMode(QToolButton::InstantPopup);
    m_filterJobs->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_filterJobs->setIcon(KIcon("view-filter"));
    m_filterJobs->setText(actionActiveJobs->text());
    toolBar2->addWidget(m_filterJobs);

    // action group to make their selection exclusive
    QActionGroup *group = new QActionGroup(this);
    // when one action is triggered it changes the model data
    connect(group, SIGNAL(triggered(QAction *)), this, SLOT(actionTriggered(QAction *)));
    // ACTIVE jobs
    actionActiveJobs->setChecked(true);
    actionActiveJobs->setData(CUPS_WHICHJOBS_ACTIVE);
    menu->addAction(actionActiveJobs);
    group->addAction(actionActiveJobs);
    // COMPLETED jobs
    actionCompletedJobs->setData(CUPS_WHICHJOBS_COMPLETED);
    menu->addAction(actionCompletedJobs);
    group->addAction(actionCompletedJobs);
    // ALL jobs
    actionAllJobs->setData(CUPS_WHICHJOBS_ALL);
    menu->addAction(actionAllJobs);
    group->addAction(actionAllJobs);

    // configure printer
    actionConfigure->setIcon(KIcon("configure"));
    toolBar2->addAction(actionConfigure);
}

void PrintQueueUi::closeEvent(QCloseEvent *event)
{
    // emits finished signal to be removed the cache
    emit finished();
    QWidget::closeEvent(event);
}

#include "PrintQueueUi.moc"
