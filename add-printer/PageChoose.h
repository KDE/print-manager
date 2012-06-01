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

#ifndef PAGE_CHOOSE_H
#define PAGE_CHOOSE_H

#include "GenericPage.h"

#include <QStackedLayout>

class ChooseIpp;
class ChooseLpd;
class ChoosePrinters;
class ChooseSamba;
class ChooseSerial;
class ChooseSocket;
class ChooseUri;
class PageChoose : public GenericPage
{
    Q_OBJECT
public:
    explicit PageChoose(const QVariantHash &args = QVariantHash(), QWidget *parent = 0);
    ~PageChoose();

    void setValues(const QVariantHash &args);
    bool isValid() const;
    bool canProceed() const;
    QVariantHash values() const;

public slots:
    void load();

private slots:
    void checkSelected();

private:
    bool m_isValid;
    QVariantHash m_args;
    QStackedLayout *m_layout;
    GenericPage    *m_currentPage;
    ChooseIpp      *m_chooseIpp;
    ChooseLpd      *m_chooseLpd;
    ChoosePrinters *m_choosePrinters;
    ChooseSamba    *m_chooseSamba;
    ChooseSerial   *m_chooseSerial;
    ChooseSocket   *m_chooseSocket;
    ChooseUri      *m_chooseUri;
};

#endif
