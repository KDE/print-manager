/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2025-2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_MODEL_H
#define PRINTER_MODEL_H

#include <KCupsPrinter.h>
#include <QAbstractItemModel>
#include <QPointer>
#include <kcups_export.h>
#include <qqmlregistration.h>

namespace ServerState
{
Q_NAMESPACE_EXPORT(KCUPS_EXPORT)
QML_ELEMENT

enum State {
    Unknown,
    Available,
    Unavailable
};
Q_ENUM_NS(State)
}

class KCupsRequest;
class IppBrowserManager;
class KCUPS_EXPORT PrinterModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * Whether the CUPS server is reachable, Unknown until the first request completed
     */
    Q_PROPERTY(ServerState::State serverState READ serverState NOTIFY serverStateChanged)
    /**
     * Whether or not to actually display the location of the printer
     *
     * Only show the location if there is more than one printer
     * and at least two distinct locations exist.  If there is only one
     * printer or 2 or more printers have the same location, this will be false
     */
    Q_PROPERTY(bool showLocations READ showLocations NOTIFY showLocationsChanged FINAL)
    /**
     * true if model only contains printers (not classes)
     */
    Q_PROPERTY(bool hasOnlyPrinters READ hasOnlyPrinters NOTIFY hasOnlyPrintersChanged FINAL)
    /**
     * Convenience to mask CUPS_PRINTER_DISCOVERED
     * set true to include discovered printers in the model
     */
    Q_PROPERTY(bool includeDiscovered READ includeDiscovered WRITE setIncludeDiscovered NOTIFY includeDiscoveredChanged FINAL)

    Q_PROPERTY(uint searchTimeout READ searchTimeout WRITE setSearchTimeout NOTIFY searchTimeoutChanged FINAL)
    Q_PROPERTY(uint filterType READ filterType WRITE setFilterType NOTIFY filterTypeChanged FINAL)
    Q_PROPERTY(uint filterMask READ filterMask WRITE setFilterMask NOTIFY filterMaskChanged FINAL)

public:
    enum Role {
        DestStatus = Qt::UserRole + 1,
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
        DestRemote,
        DestUri,
        DestUriSupported,
        DestMemberNames,
        DestIsDiscovered,
        DestMoreInfo,
        DestSupplyInfoUri
    };
    Q_ENUM(Role)

    explicit PrinterModel(QObject *parent = nullptr);

    ServerState::State serverState() const;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void clear();

    Q_INVOKABLE void update();

    bool serverUnavailable() const;
    bool showLocations() const;
    bool hasOnlyPrinters() const;

    bool includeDiscovered() const;
    void setIncludeDiscovered(bool newIncludeDiscovered);

    uint searchTimeout() const;
    void setSearchTimeout(uint timeout);

    uint filterType() const;
    void setFilterType(uint filterType);

    uint filterMask() const;
    void setFilterMask(uint filterMask);

Q_SIGNALS:
    void serverStateChanged();
    void error(int lastError, const QString &errorTitle, const QString &errorMsg);
    void showLocationsChanged();
    void hasOnlyPrintersChanged();
    void includeDiscoveredChanged();
    void searchTimeoutChanged();
    void filterTypeChanged();
    void filterMaskChanged();

private:
    void printerRemoved(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);
    void printerStateChanged(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);
    void printerStopped(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);
    void printerRestarted(const QString &text,
                          const QString &printerUri,
                          const QString &printerName,
                          uint printerState,
                          const QString &printerStateReasons,
                          bool printerIsAcceptingJobs);
    void printerShutdown(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs);
    void printerModified(const QString &text,
                         const QString &printerUri,
                         const QString &printerName,
                         uint printerState,
                         const QString &printerStateReasons,
                         bool printerIsAcceptingJobs);
    void printerAdded(const QString &text,
                      const QString &printerUri,
                      const QString &printerName,
                      uint printerState,
                      const QString &printerStateReasons,
                      bool printerIsAcceptingJobs);
    void serverChanged(const QString &text);

private:
    ServerState::State m_serverState = ServerState::Unknown;
    bool m_showLocations = true;
    bool m_hasOnlyPrinters = true;
    uint m_searchTimeout = 3000;
    uint m_filterType = 0; // include all
    uint m_filterMask = CUPS_PRINTER_DISCOVERED; // mask out discovered
    bool m_includeDiscovered = false;
    KCupsPrinters m_printers;

    void setServerState(ServerState::State state);
    void insertUpdateFinished(KCupsRequest *request);
    void insertUpdatePrinter(const QString &printerName);
    void updateDisplayHints();

    void getDestsFinished(KCupsRequest *request);
    int findIndex(const QString &destName);
    QString destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const;

    using RequestFunc = void (PrinterModel::*)(KCupsRequest *);
    /**
     * Set up a request, handles error and deletelater
     * @param func runs when KCupsRequest::finished
     */
    KCupsRequest *setupRequest(RequestFunc func = nullptr);
    void setModelItem(const KCupsPrinter &printer);
    void getIppDirectData(QList<KCupsPrinter>::iterator printer);
};

#endif // PRINTER_MODEL_H
