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

#ifndef PRINTER_MODEL_H
#define PRINTER_MODEL_H

#include <QStandardItemModel>
#include <QTimer>

#include <KCupsPrinter.h>

class PrinterModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(JobAction)
    Q_ENUMS(Role)
public:
    typedef enum {
        DestStatus = Qt::UserRole,
        DestName,
        DestIsDefault,
        DestIsShared,
        DestIsAcceptingJobs,
        DestIsClass,
        DestLocation,
        DestDescription,
        DestKind,
        DestType,
        DestCommands,
        DestMarkerChangeTime,
        DestMarkers
    } Role;

    typedef enum {
        Cancel,
        Hold,
        Release,
        Move
    } JobAction;

    explicit PrinterModel(WId parentId, QObject *parent = 0);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
    void update();
    void getDestsFinished();

signals:
    void error(int lastError, const QString &errorTitle, const QString &errorMsg);

private slots:
    void insertUpdatePrinter(const QString &printerName);
    void insertUpdatePrinter(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);
    void insertUpdatePrinterFinished();
    void printerRemoved(const QString &printerName);
    void printerRemoved(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerStateChanged(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerStopped(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerRestarted(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerShutdown(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerModified(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);

private:
    WId m_parentId;
    QStringList m_attributes;

    int destRow(const QString &destName);
    void insertDest(int pos, const KCupsPrinter &printer);
    void updateDest(QStandardItem *item, const KCupsPrinter &printer);

    QString destStatus(KCupsPrinter::Status state, const QString &message) const;
};

#endif // PRINTER_MODEL_H
