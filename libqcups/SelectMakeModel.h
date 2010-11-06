/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#ifndef SELECT_MAKE_MODEL_H
#define SELECT_MAKE_MODEL_H

#include <QWidget>
#include <QSortFilterProxyModel>

#include <KPixmapSequenceOverlayPainter>
#include <kdemacros.h>

namespace Ui {
    class SelectMakeModel;
};

namespace QCups {
    class Result;
};

class KDE_EXPORT SelectMakeModel : public QWidget
{
    Q_OBJECT
public:
    SelectMakeModel(QWidget *parent = 0);
    ~SelectMakeModel();

    void setMakeModel(const QString &make, const QString &makeAndModel);
    QString selectedPPDName() const;
    QString selectedMakeAndModel() const;

public slots:
    void checkChanged();
    void ppdsLoaded();

signals:
    void changed(bool);

private slots:
    void on_makeFilterKCB_editTextChanged(const QString &text);

private:
    Ui::SelectMakeModel *ui;
    QCups::Result *m_result;
    KPixmapSequenceOverlayPainter *m_busySeq;
    QString m_selectedPPDName, m_selectedMakeAndModel;
    QString m_make, m_makeAndModel;
    QSortFilterProxyModel *m_model;
};


#endif
