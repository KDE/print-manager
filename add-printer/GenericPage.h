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

#ifndef GENERIC_PAGE_H
#define GENERIC_PAGE_H

#include <QWidget>
#include <QHash>
#include <QVariant>

#define DEVICE_ID              "device-id"
#define DEVICE_URI             "device-uri"
#define DEVICE_MAKE_MODEL      "device-make-model"
#define DEVICE_INFO            "device-info"
#define DEVICE_LOCATION        "device-location"
#define ADDING_PRINTER         "add-new-printer"
#define PPD_NAME               "ppd-name"
#define PRINTER_NAME           "printer-name"
#define PRINTER_LOCATION       "printer-location"
#define PRINTER_INFO           "printer-info"
#define PRINTER_MAKE_AND_MODEL "printer-make-and-model"
#define MEMBER_URIS            "member-uris"
#define FILENAME               "filename"

class GenericPage : public QWidget
{
    Q_OBJECT
public:
    GenericPage(QWidget *parent = 0);
    virtual bool canProceed() const { return true; };
    virtual bool hasChanges() const { return false; };
    virtual bool isValid() const { return true; };
    virtual void setValues(const QVariantHash &args);
    virtual QVariantHash values() const;

signals:
    void allowProceed(bool allow);
    void proceed();

protected:
    QHash<QString, QVariant> m_args;
};

#endif
