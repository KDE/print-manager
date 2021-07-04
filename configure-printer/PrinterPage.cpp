/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrinterPage.h"

#include <QHash>
#include <QVariant>

PrinterPage::PrinterPage(QWidget *parent)
 : QWidget(parent)
{
}

QVariantHash PrinterPage::modifiedValues() const
{
    return QVariantHash();
}

void PrinterPage::setRemote(bool)
{
}

#include "moc_PrinterPage.cpp"
