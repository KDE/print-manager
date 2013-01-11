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

#ifndef PRINT_QUEUE_UI_H
#define PRINT_QUEUE_UI_H

#include <KDialog>
#include <QModelIndex>
#include <QToolButton>

namespace Ui {
    class PrintQueueUi;
}

class KCupsPrinter;
class PrintQueueSortFilterProxyModel;
class PrintQueueModel;
class PrintQueueUi : public KDialog
{
    Q_OBJECT
public:
    explicit PrintQueueUi(const KCupsPrinter &printer, QWidget *parent = 0);
    ~PrintQueueUi();

signals:
    void finished();

public slots:
    void update();

private slots:
    void updatePrinter(const QString &printer);
    void updatePrinter(const QString &text,
                       const QString &printerUri,
                       const QString &printerName,
                       uint printerState,
                       const QString &printerStateReasons,
                       bool printerIsAcceptingJobs);
    void whichJobsIndexChanged(int index);
    void pausePrinter();
    void configurePrinter();

    void cancelJob();
    void holdJob();
    void resumeJob();

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void updateButtons();
    void showContextMenu(const QPoint &point);
    void showHeaderContextMenu(const QPoint &point);
    void getAttributesFinished();

private:
    void closeEvent(QCloseEvent *event);
    void setupButtons();
    void setState(int state, const QString &message);
    void modifyJob(int action, const QString &destName = QString());

    Ui::PrintQueueUi *ui;
    QToolButton *m_filterJobs;
    PrintQueueSortFilterProxyModel *m_proxyModel;
    PrintQueueModel *m_model;
    QString m_destName;
    QString m_title;
    bool m_isClass;
    bool m_preparingMenu;
    QPixmap m_printerIcon;
    QPixmap m_pauseIcon;
    QPixmap m_startIcon;
    QPixmap m_warningIcon;
    bool m_printerPaused;
    char m_lastState;
};

#endif
