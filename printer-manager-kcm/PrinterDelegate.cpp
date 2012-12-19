/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2008-2010 Daniel Nicoletti <dantti12@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "PrinterDelegate.h"
#include "PrinterModel.h"

#include <cmath>

#include <QtGui/QPainter>

#include <KDebug>
#include <KIconLoader>

PrinterDelegate::PrinterDelegate(QObject *parent)
: QStyledItemDelegate(parent)
{
    m_mainIconSize = IconSize(KIconLoader::Dialog); // 32
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
    if (!index.isValid() && index.column() == 0){
      return;
    }

    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    int left = option.rect.left();
    int top = option.rect.top();
    int width = option.rect.width();

    bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);

    // selects the mode to paint the icon based on the info field
    QIcon::Mode iconMode;
    if (option.state & QStyle::State_MouseOver) {
        iconMode = QIcon::Active;
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

    QPixmap pixmap(option.rect.size());
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());

    QLinearGradient gradient;

    // Painting

    // Text
    int textInner = 2 * m_universalPadding + m_mainIconSize;
    const int itemHeight = calcItemHeight(option);

    QString status = index.data(PrinterModel::DestStatus).toString();
    QString description = index.data(Qt::DisplayRole).toString();

    p.setPen(foregroundColor);
    p.setFont(local_option_title.font);
    p.drawText(left + (leftToRight ? textInner : 0),
                top,
                width - textInner,
                itemHeight / 2,
                Qt::AlignBottom | Qt::AlignLeft,
                description);

    p.setFont(local_option_normal.font);
    p.drawText(left + (leftToRight ? textInner : 0) + 10,
                (top + itemHeight / 2) + 1,
                width - textInner,
                itemHeight / 2,
                Qt::AlignTop | Qt::AlignLeft,
                status);

    // Main icon
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    // if we have a package that mean we
    // can try to get a better icon
    icon.paint(&p,
               leftToRight ? left + m_universalPadding : left + width - m_universalPadding - m_mainIconSize,
               top + m_universalPadding,
               m_mainIconSize,
               m_mainIconSize,
               Qt::AlignCenter,
               iconMode);

    // Counting the number of emblems for this item
    int emblemCount = 1;

    // Gradient part of the background - fading of the text at the end
    if (leftToRight) {
        gradient = QLinearGradient(left + width - m_universalPadding - m_fadeLength, 0,
                left + width - m_universalPadding, 0);
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, Qt::transparent);
    } else {
        gradient = QLinearGradient(left + m_universalPadding, 0,
                left + m_universalPadding + m_fadeLength, 0);
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, Qt::white);
    }

    QRect paintRect = option.rect;
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(paintRect, gradient);

    if (leftToRight) {
        gradient.setStart(left + width
                - emblemCount * (m_universalPadding + m_emblemIconSize) - m_fadeLength, 0);
        gradient.setFinalStop(left + width
                - emblemCount * (m_universalPadding + m_emblemIconSize), 0);
    } else {
        gradient.setStart(left + m_universalPadding
                + emblemCount * (m_universalPadding + m_emblemIconSize), 0);
        gradient.setFinalStop(left + m_universalPadding
                + emblemCount * (m_universalPadding + m_emblemIconSize) + m_fadeLength, 0);
    }
    paintRect.setHeight(m_universalPadding + m_mainIconSize / 2);
    p.fillRect(paintRect, gradient);

    painter->drawPixmap(option.rect.topLeft(), pixmap);
}

QSize PrinterDelegate::sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index ) const
{
    int width = (index.column() == 0) ? index.data(Qt::SizeHintRole).toSize().width() : m_favIconSize + 2 * m_universalPadding;
    QSize ret;

    return QSize(width, calcItemHeight(option));
}

#include "PrinterDelegate.moc"
