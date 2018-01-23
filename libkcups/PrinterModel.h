/***************************************************************************
 *   Copyright (C) 2010-2018 by Daniel Nicoletti                           *
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

class Q_DECL_EXPORT PrinterModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(JobAction)
    Q_ENUMS(Role)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool serverUnavailable READ serverUnavailable NOTIFY serverUnavailableChanged)
public:
    enum Role {
        DestStatus = Qt::UserRole,
        DestState,
        DestName,
        DestIsDefault,
        DestIsShared,
        DestIsAcceptingJobs,
        DestIsPaused,
        DestIsClass,
        DestLocation,
        DestDescription,
        DestKind,
        DestType,
        DestCommands,
        DestMarkerChangeTime,
        DestMarkers,
        DestIconName,
        DestRemote
    };

    enum JobAction {
        Cancel,
        Hold,
        Release,
        Move
    };

    explicit PrinterModel(QObject *parent = 0);

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    int count() const;
    bool serverUnavailable() const;

    Q_INVOKABLE void pausePrinter(const QString &printerName);
    Q_INVOKABLE void resumePrinter(const QString &printerName);
    Q_INVOKABLE void rejectJobs(const QString &printerName);
    Q_INVOKABLE void acceptJobs(const QString &printerName);

public slots:
    void update();
    void getDestsFinished();
    void slotCountChanged();

signals:
    void countChanged(int count);
    void serverUnavailableChanged(bool unavailable);
    void error(int lastError, const QString &errorTitle, const QString &errorMsg);

private slots:
    void insertUpdatePrinterName(const QString &printerName);
    void insertUpdatePrinter(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);
    void insertUpdatePrinterFinished();
    void printerRemovedName(const QString &printerName);
    void printerRemoved(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerStateChanged(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerStopped(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerRestarted(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerShutdown(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void printerModified(const QString &text, const QString &printerUri, const QString &printerName, uint printerState, const QString &printerStateReasons, bool printerIsAcceptingJobs);
    void serverChanged(const QString &text);

private:
    WId m_parentId;
    QStringList m_attributes;
    bool m_unavailable;

    int destRow(const QString &destName);
    void insertDest(int pos, const KCupsPrinter &printer);
    void updateDest(QStandardItem *item, const KCupsPrinter &printer);

    QString destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const;
    void clear();
};

#endif // PRINTER_MODEL_H
