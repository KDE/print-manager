/*
    SPDX-FileCopyrightText: 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
    SPDX-FileCopyrightText: 2008-2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "PrinterDelegate.h"
#include <PrinterModel.h>

#include <cmath>

#include <QPainter>
#include <QStyleOption>

PrinterDelegate::PrinterDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_mainIconSize = 32;
    m_favIconSize = m_mainIconSize * 0.75; // 24
    m_emblemIconSize = m_mainIconSize / 4; // 8
    m_universalPadding = m_mainIconSize / 8; // 4
    m_fadeLength = m_mainIconSize / 2; // 16
}

int PrinterDelegate::calcItemHeight(const QStyleOptionViewItem &option) const
{
    // Painting main column
    QStyleOptionViewItem local_option_title(option);
    QStyleOptionViewItem local_option_normal(option);

    local_option_normal.font.setPointSize(local_option_normal.font.pointSize() - 1);

    int textHeight = QFontInfo(local_option_title.font).pixelSize() + QFontInfo(local_option_normal.font).pixelSize();
    return qMax(textHeight, m_mainIconSize) + 2 * m_universalPadding;
}


void PrinterDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid() && index.column() == 0) {
        return;
    }

    QStyleOptionViewItem opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    int left = option.rect.left();
    int top = option.rect.top();
    int width = option.rect.width();

    bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);
    bool isPaused = index.data(PrinterModel::DestIsPaused).toBool();

    // selects the mode to paint the icon based on the info field
    QIcon::Mode iconMode;
    if (isPaused) {
        iconMode = QIcon::Disabled;
    } else if (option.state & QStyle::State_MouseOver) {
        iconMode = QIcon::Active;
    } else if (option.state.testFlag(QStyle::State_Selected)) {
        iconMode = QIcon::Selected;
    } else {
        iconMode = QIcon::Normal;
    }

    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
                             option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);

    // Painting main column
    QStyleOptionViewItem local_option_title(option);
    QStyleOptionViewItem local_option_normal(option);

    local_option_normal.font.setPointSize(local_option_normal.font.pointSize() - 1);
    local_option_title.font.setBold(index.data(PrinterModel::DestIsDefault).toBool());

    // Painting

    // Text
    int textInner = 2 * m_universalPadding + m_mainIconSize;
    const int itemHeight = calcItemHeight(option);

    QString status = index.data(PrinterModel::DestStatus).toString();
    QString description = index.data(Qt::DisplayRole).toString();

    painter->setPen(foregroundColor);
    painter->setFont(local_option_title.font);
    painter->drawText(left + (leftToRight ? textInner : 0),
               top,
               width - textInner,
               itemHeight / 2,
               Qt::AlignBottom | Qt::AlignLeft,
               local_option_title.fontMetrics.elidedText(description, Qt::ElideRight, width - textInner));

    painter->setFont(local_option_normal.font);
    painter->drawText(left + (leftToRight ? textInner : 0) + 10,
               (top + itemHeight / 2) + 1,
               width - textInner,
               itemHeight / 2,
               Qt::AlignTop | Qt::AlignLeft,
               local_option_normal.fontMetrics.elidedText(status, Qt::ElideRight, width - textInner));

    // Main icon
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    // if we have a package that mean we
    // can try to get a better icon
    icon.paint(painter,
               leftToRight ? left + m_universalPadding : left + width - m_universalPadding - m_mainIconSize,
               top + m_universalPadding,
               m_mainIconSize,
               m_mainIconSize,
               Qt::AlignCenter,
               iconMode);
}

QSize PrinterDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index ) const
{
    int width = (index.column() == 0) ? index.data(Qt::SizeHintRole).toSize().width() : m_favIconSize + 2 * m_universalPadding;
    QSize ret;

    return QSize(width, calcItemHeight(option));
}

#include "moc_PrinterDelegate.cpp"
