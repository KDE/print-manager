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

#ifndef PRINTER_OPTIONS_H
#define PRINTER_OPTIONS_H

#include "PrinterPage.h"

#include <cups/ppd.h>
#include <QTextCodec>
#include <QAbstractButton>
#include <QHash>

namespace Ui {
    class PrinterOptions;
}

class PrinterOptions : public PrinterPage
{
    Q_OBJECT
public:
    explicit PrinterOptions(const QString &destName, bool isClass, bool isRemote, QWidget *parent = 0);
    ~PrinterOptions();

    bool hasChanges();

    QString currentMake() const;
    QString currentMakeAndModel() const;
    void reloadPPD();

    void save();

private slots:
    void on_autoConfigurePB_clicked();
    void currentIndexChangedCB(int index);
    void radioBtClicked(QAbstractButton *button);

private:
    QWidget* pickBoolean(ppd_option_t *option, const QString &keyword, QWidget *parent) const;
    QWidget* pickMany(ppd_option_t *option, const QString &keyword, QWidget *parent) const;
    QWidget* pickOne(ppd_option_t *option, const QString &keyword, QWidget *parent) const;
    const char* getVariable(const char *name) const;
    char * get_option_value(ppd_file_t *ppd, const char *name, char *buffer, size_t bufsize) const;
    static double get_points(double number, const char *uval);

    void createGroups();

    Ui::PrinterOptions *ui;
    QString m_destName;
    bool m_isClass;
    bool m_isRemote;
    QString m_filename;
    ppd_file_t *m_ppd;
    int m_changes;
    QTextCodec *m_codec;
    QHash<QString, QObject*> m_customValues;
    QString m_make, m_makeAndModel;
};

#endif
