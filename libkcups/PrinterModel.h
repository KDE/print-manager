/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_MODEL_H
#define PRINTER_MODEL_H

#include <QStandardItemModel>
#include <QTimer>

#include <KCupsPrinter.h>

class KCupsRequest;
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

    explicit PrinterModel(QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int count() const;
    bool serverUnavailable() const;

    virtual QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void pausePrinter(const QString &printerName);
    Q_INVOKABLE void resumePrinter(const QString &printerName);
    Q_INVOKABLE void rejectJobs(const QString &printerName);
    Q_INVOKABLE void acceptJobs(const QString &printerName);

public slots:
    void update();
    void getDestsFinished(KCupsRequest *request);
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
    void insertUpdatePrinterFinished(KCupsRequest *request);
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
    QHash<int, QByteArray> m_roles;
    bool m_unavailable = true;

    int destRow(const QString &destName);
    void insertDest(int pos, const KCupsPrinter &printer);
    void updateDest(QStandardItem *item, const KCupsPrinter &printer);

    QString destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const;
    void clear();
};

#endif // PRINTER_MODEL_H
