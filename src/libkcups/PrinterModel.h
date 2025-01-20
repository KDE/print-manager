/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2025-2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_MODEL_H
#define PRINTER_MODEL_H

#include <KCupsPrinter.h>
#include <kcups_export.h>
#include <QStandardItemModel>
#include <qqmlregistration.h>

class KCupsRequest;
class KCUPS_EXPORT PrinterModel : public QStandardItemModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool serverUnavailable READ serverUnavailable NOTIFY serverUnavailableChanged FINAL)
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
     * set true to include discovered printers in the model
     */
    Q_PROPERTY(bool includeDiscovered READ includeDiscovered WRITE setIncludeDiscovered NOTIFY includeDiscoveredChanged FINAL)
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
        DestRemote,
        DestUri,
        DestUriSupported,
        DestMemberNames,
        DestIsDiscovered
    };
    Q_ENUM(Role)

    enum JobAction {
        Cancel,
        Hold,
        Release,
        Move
    };
    Q_ENUM(JobAction)

    explicit PrinterModel(QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void pausePrinter(const QString &printerName);
    Q_INVOKABLE void resumePrinter(const QString &printerName);
    Q_INVOKABLE void update();

    bool serverUnavailable() const;
    bool showLocations() const;
    bool hasOnlyPrinters() const;

    bool includeDiscovered() const;
    void setIncludeDiscovered(bool newIncludeDiscovered);

Q_SIGNALS:
    void serverUnavailableChanged(bool unavailable);
    void error(int lastError, const QString &errorTitle, const QString &errorMsg);
    void showLocationsChanged();
    void hasOnlyPrintersChanged();
    void includeDiscoveredChanged();

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

    QHash<int, QByteArray> m_roles;
    bool m_unavailable = true;
    bool m_showLocations = true;
    bool m_hasOnlyPrinters = true;
    QStringList m_attrs;
    uint m_searchTimeout = 5000;
    uint m_filterType = 0; // include all
    uint m_filterMask = CUPS_PRINTER_DISCOVERED; // mask out discovered
    bool m_includeDiscovered = false;

    void newDestination(const QVariantMap &destination);
    void insertUpdatePrinter(const QString &printerName);
    void updateDisplayHints();

    void getDestsFinished(KCupsRequest *request);
    int destRow(const QString &destName);
    QString destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const;
    void updateDest(QStandardItem *item, const KCupsPrinter &printer);

    typedef std::function<void(PrinterModel *, KCupsRequest *)> RequestFunc;
    /**
     * Set up a request, handles error and deletelater
     * @param func run when the request is finished
     */
    KCupsRequest *setupRequest(RequestFunc func = nullptr);
};

#endif // PRINTER_MODEL_H
