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

#ifndef CLASS_LIST_WIDGET_H
#define CLASS_LIST_WIDGET_H

#include <QStandardItemModel>
#include <QListView>
#include <QTimer>

#include <KPixmapSequenceOverlayPainter>

class KCupsRequest;
class KDE_EXPORT ClassListWidget : public QListView
{
    Q_OBJECT
    Q_PROPERTY(QString selectedPrinters READ selectedPrinters WRITE setSelectedPrinters USER true)
    Q_PROPERTY(bool showClasses READ showClasses WRITE setShowClasses)
public:
    explicit ClassListWidget(QWidget *parent = 0);
    ~ClassListWidget();

    bool hasChanges();
    void setPrinter(const QString &printer);
    QString selectedPrinters() const;
    void setSelectedPrinters(const QString &selected);
    bool showClasses() const;
    void setShowClasses(bool enable);
    QStringList currentSelected(bool uri) const;

signals:
    void changed(bool changed);
    void changed(const QString &selected);

private slots:
    void init();
    void loadFinished();
    void modelChanged();

private:
    void updateItemState(QStandardItem *item) const;

    QString m_printerName;
    QStringList m_selectedPrinters;
    KPixmapSequenceOverlayPainter *m_busySeq;
    KCupsRequest *m_request;
    bool m_changed;
    bool m_showClasses;
    QStandardItemModel *m_model;
    QTimer m_delayedInit;
};

#endif
