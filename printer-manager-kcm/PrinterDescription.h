/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINTER_DESCRIPTION_H
#define PRINTER_DESCRIPTION_H

#include <QWidget>

#include <KCupsRequest.h>


namespace Ui {
class PrinterDescription;
}
class PrinterDescription : public QWidget
{
    Q_OBJECT
public:
    explicit PrinterDescription(QWidget *parent = nullptr);
    ~PrinterDescription();

    void setPrinterIcon(const QIcon &icon);
    void setDestName(const QString &name, const QString &description, bool isClass, bool singlePrinter);
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

    void requestFinished(KCupsRequest *request);

private:
    Ui::PrinterDescription *ui;
    QString m_destName;
    bool m_isClass = false;
    bool m_isShared;
    bool m_globalShared = false;
    QStringList m_commands;
    QPixmap m_printerIcon;
    int m_markerChangeTime = 0;
    QVariantHash m_markerData;
    int m_layoutEnd;
};

#endif
