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

#ifndef PRINTER_DESCRIPTION_H
#define PRINTER_DESCRIPTION_H

#include <QWidget>

#include <KCupsRequest.h>

class QToolButton;
class QSortFilterProxyModel;

namespace Ui {
    class PrinterDescription;
}
class PrintQueueModel;
class PrinterDescription : public QWidget
{
    Q_OBJECT
public:
    explicit PrinterDescription(QWidget *parent = 0);
    ~PrinterDescription();

    void setPrinterIcon(const QIcon &icon);
    void setDestName(const QString &name, const QString &description, bool isClass);
    void setDestStatus(const QString &status);
    void setLocation(const QString &location);
    void setKind(const QString &kind);
    void setIsDefault(bool isDefault);
    void setIsShared(bool isShared);
    void setAcceptingJobs(bool accepting);
    void setCommands(const QStringList &commands);

    void setMarkers(const QVariantHash &data);

    QString destName() const;

public slots:
    void enableShareCheckBox(bool enable);

signals:
    void updateNeeded();

private slots:
    void on_configurePB_clicked();
    void on_openQueuePB_clicked();
    void on_defaultCB_clicked();
    void on_sharedCB_clicked();
    void on_rejectPrintJobsCB_clicked();

    void on_actionPrintTestPage_triggered(bool checked);
    void on_actionCleanPrintHeads_triggered(bool checked);
    void on_actionPrintSelfTestPage_triggered(bool checked);

    void requestFinished();

private:
    Ui::PrinterDescription *ui;
    QString m_destName;
    bool m_isClass;
    bool m_isShared;
    bool m_globalShared;
    QStringList m_commands;
    QPixmap m_printerIcon;
    QPixmap m_pauseIcon;
    QPixmap m_startIcon;
    QPixmap m_warningIcon;
    int m_markerChangeTime;
    QVariantHash m_markerData;
    int m_layoutEnd;
};

#endif
