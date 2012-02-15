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

#include "SupplyLevels.h"

#include <KLocale>
#include <QScrollArea>
#include <QFormLayout>
#include <QProgressBar>
#include <QLabel>

#include <KDebug>

// Thanks to Luiz Vitor and his MacBook Pro that had an Epson printer which
// reported the following data
// ("marker-names",  QVariant(QStringList, ("Cyan", "Yellow", "Magenta", "Black")))
// ("marker-colors",  QVariant(QStringList, ("#00ffff", "#ffff00", "#ff00ff", "#000000")))
// ("marker-levels",  QVariant(QList<int>, ))
// ("marker-types",  QVariant(QStringList, ("inkCartridge", "inkCartridge", "inkCartridge", "inkCartridge")))

SupplyLevels::SupplyLevels(const QVariantHash &args, QWidget *parent)
 : KDialog(parent)
{
    setModal(true);
    setButtons(KDialog::Close);
    setWindowTitle(i18n("Supply Levels"));

    KConfig config("print-manager");
    KConfigGroup supplyLevels(&config, "SupplyLevels");
    restoreDialogSize(supplyLevels);

    int size = args["marker-names"].toStringList().size();
    if (size != args["marker-levels"].value<QList<int> >().size() ||
        size != args["marker-colors"].toStringList().size() ||
        size != args["marker-types"].toStringList().size()) {
        setMainWidget(new QLabel(i18n("Invalid data")));
        return;
    }

    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *scrollAreaWidgetContents = new QWidget(scrollArea);
    scrollArea->setWidget(scrollAreaWidgetContents);
    scrollArea->setFrameShape(QFrame::NoFrame);
    setMainWidget(scrollArea);
    QFormLayout *layout = new QFormLayout(scrollAreaWidgetContents);
    layout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    mainWidget()->setLayout(layout);

    // Create a colored progress bar for each marker
    for (int i = 0; i < size; i++) {
        QProgressBar *pb = new QProgressBar;
        pb->setValue(args["marker-levels"].value<QList<int> >().at(i));
        pb->setTextVisible(false);
        pb->setMaximumHeight(15);
        QPalette palette = pb->palette();
        palette.setColor(QPalette::Active,
                         QPalette::Highlight,
                         QColor(args["marker-colors"].toStringList().at(i)));
        pb->setPalette(palette);
        layout->addRow(args["marker-names"].toStringList().at(i), pb);
    }
}

SupplyLevels::~SupplyLevels()
{
    KConfig config("print-manager");
    KConfigGroup supplyLevels(&config, "SupplyLevels");
    saveDialogSize(supplyLevels);
}

#include "SupplyLevels.moc"
