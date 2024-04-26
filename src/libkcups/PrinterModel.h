/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_MODEL_H
#define PRINTER_MODEL_H

#include <QStandardItemModel>
#include <qqmlregistration.h>

#include <KCupsPrinter.h>
#include <kcupslib_export.h>

class KCupsRequest;
class KCUPSLIB_EXPORT PrinterModel : public QStandardItemModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int searchTimeout READ searchTimeout WRITE setSearchTimeout CONSTANT FINAL)
    Q_PROPERTY(bool searchIncludeDiscovered READ searchIncludeDiscovered WRITE setSearchIncludeDiscovered NOTIFY searchIncludeDiscoveredChanged FINAL)

    Q_PROPERTY(bool serverUnavailable READ serverUnavailable NOTIFY serverUnavailableChanged)
    /**
     * Whether or not to actually display the location of the printer
     *
     * Only show the location if there is more than one printer
     * and at least two distinct locations exist.  If there is only one
     * printer or 2 or more printers have the same location, this will be false
     */
    Q_PROPERTY(bool displayLocationHint READ displayLocationHint NOTIFY displayLocationHintChanged)
    /**
     * true if model only contains printers (not classes)
     */
    Q_PROPERTY(bool printersOnly READ printersOnly NOTIFY printersOnlyChanged)

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
        DestMemberNames
    };
    Q_ENUM(Role)

    enum JobAction { Cancel, Hold, Release, Move };
    Q_ENUM(JobAction)

    explicit PrinterModel(QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool serverUnavailable() const;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void pausePrinter(const QString &printerName);
    Q_INVOKABLE void resumePrinter(const QString &printerName);
    Q_INVOKABLE void rejectJobs(const QString &printerName);
    Q_INVOKABLE void acceptJobs(const QString &printerName);
    Q_INVOKABLE void update();

    bool displayLocationHint() const;
    int searchTimeout() const;
    void setSearchTimeout(int newSearchTimeout);

    cups_ptype_t filterType() const;
    void setFilterType(cups_ptype_t newFilterType);

    cups_ptype_t filterMask() const;
    void setFilterMask(cups_ptype_t newFilterMask);

    bool searchIncludeDiscovered() const;
    void setSearchIncludeDiscovered(bool newSearchIncludeDiscovered);

signals:
    void serverUnavailableChanged(bool unavailable);
    void error(int lastError, const QString &errorTitle, const QString &errorMsg);
    void displayLocationHintChanged();
    void printersOnlyChanged();

    void searchIncludeDiscoveredChanged();

private slots:
    void getDestsFinished(KCupsRequest *request);

    void getPrinterAttributes(const QString &printer);

    void insertPrinter(const QString &text,
                       const QString &printerUri,
                       const QString &printerName,
                       uint printerState,
                       const QString &printerStateReasons,
                       bool printerIsAcceptingJobs);

    void updatePrinterState(const QString &text,
                     const QString &printerUri,
                     const QString &printerName,
                     uint printerState,
                     const QString &printerStateReasons,
                     bool printerIsAcceptingJobs);

    void updatePrinter(const QString &text,
                             const QString &printerUri,
                             const QString &printerName,
                             uint printerState,
                             const QString &printerStateReasons,
                             bool printerIsAcceptingJobs);

    void removePrinter(const QString &text,
                        const QString &printerUri,
                        const QString &printerName,
                        uint printerState,
                        const QString &printerStateReasons,
                        bool printerIsAcceptingJobs);

    void serverChanged(const QString &text);

private:
    QHash<int, QByteArray> m_roles;
    bool m_unavailable = true;
    bool m_displayLocationHint = true;
    bool m_printersOnly = true;
    int m_searchTimeout = 5000;
    bool m_searchIncludeDiscovered = false;

    void setDisplayLocationHint();
    int destRow(const QString &destName);
    void updateDest(QStandardItem *item, const KCupsPrinter &printer);

    QString destStatus(KCupsPrinter::Status state, const QString &message, bool isAcceptingJobs) const;
    bool printersOnly() const;
    QStringList m_attrs;
    void gotDevice(const QVariantMap &device);
    void setPrintersOnly();
};

#endif // PRINTER_MODEL_H
