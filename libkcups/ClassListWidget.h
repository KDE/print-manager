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

#include <KPixmapSequenceOverlayPainter>

class KCupsRequest;
class KDE_EXPORT ClassListWidget : public QListView
{
    Q_OBJECT
public:
    explicit ClassListWidget(QWidget *parent = 0);
    ~ClassListWidget();

    bool hasChanges();
    QStringList selectedDests() const;

    void reload(const QString &destName, const QStringList &memberNames = QStringList());

signals:
    void changed(bool changed);

private slots:
    void loadFinished();
    void modelChanged();

private:
    KPixmapSequenceOverlayPainter *m_busySeq;
    KCupsRequest *m_request;
    QStringList m_selectedDests;
    bool m_changed;
    QStandardItemModel *m_model;
};

#endif
