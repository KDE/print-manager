/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2026 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrintManagerKded.h"
#include "MarkerLevelChecker.h"
#ifdef LIBCUPS_VERSION_2
#include "NewPrinterNotification.h"
#endif
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PrintManagerKded, "printmanager.json")

PrintManagerKded::PrintManagerKded(QObject *parent, const QVariantList &args)
    : KDEDModule(parent)
{
    Q_UNUSED(args)

#ifdef LIBCUPS_VERSION_2
    new NewPrinterNotification(this);
#endif
    new MarkerLevelChecker(this);
}

PrintManagerKded::~PrintManagerKded()
{
}

#include "PrintManagerKded.moc"

#include "moc_PrintManagerKded.cpp"
