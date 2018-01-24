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

#ifndef ADD_PRINTER_ASSISTANT_H
#define ADD_PRINTER_ASSISTANT_H

#include <KAssistantDialog>

#include <KPixmapSequenceOverlayPainter>

class AddPrinterAssistant : public KAssistantDialog
{
    Q_OBJECT
public:
    AddPrinterAssistant();
    virtual ~AddPrinterAssistant();

    void initAddPrinter(const QString &printer = QString(), const QString &deviceId = QString());
    void initAddClass();
    void initChangePPD(const QString &printer, const QString &deviceUri, const QString &makeAndModel);

public slots:
    void back() Q_DECL_OVERRIDE;
    void next() Q_DECL_OVERRIDE;
    void enableNextButton(bool enable);
    void enableFinishButton(bool enable);
    void slotFinishButtonClicked();

private:
    void next(KPageWidgetItem *currentPage);
    void setCurrentPage(KPageWidgetItem *page);
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;

    KPageWidgetItem *m_devicesPage = nullptr;
    KPageWidgetItem *m_chooseClassPage = nullptr;
    KPageWidgetItem *m_choosePPDPage = nullptr;
    KPageWidgetItem *m_addPrinterPage = nullptr;
    KPixmapSequenceOverlayPainter *m_busySeq;
};

#endif
