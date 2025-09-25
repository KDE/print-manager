/*
    SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINT_QUEUE_H
#define PRINT_QUEUE_H

#include <QObject>

class QQuickWindow;
class QQmlApplicationEngine;

class PrintQueue : public QObject
{
    Q_OBJECT

public:
    explicit PrintQueue(const QString &printerName, QObject *parent = nullptr);
    ~PrintQueue() override;

    QQuickWindow *mainWindow() const;

public Q_SLOTS:
    void init(const QString &printerName);
    void authenticateJob(const QString &printerName, int jobId, bool isClass);

Q_SIGNALS:
    void showQueue(const QString &printerName);

private:
    std::unique_ptr<QQmlApplicationEngine, QScopedPointerDeleteLater> m_engine;
    std::unique_ptr<QQuickWindow> m_mainWindow;
};

#endif
