/*
    SPDX-FileCopyrightText: 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
    SPDX-FileCopyrightText: 2008-2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PRINTER_DELEGATE_H
#define PRINTER_DELEGATE_H

#include <QApplication>
#include <QStyledItemDelegate>

class QPainter;

/**
 * Delegate for displaying the printers
 */
class PrinterDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PrinterDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_favIconSize;
    int m_emblemIconSize;
    int m_universalPadding;
    int m_fadeLength;
    int m_mainIconSize;
    int calcItemHeight(const QStyleOptionViewItem &option) const;
};

#endif
