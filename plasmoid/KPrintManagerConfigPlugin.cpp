/***************************************************************************
 *   Copyright (C) 2012-2013 by Daniel Nicoletti <dantti12@gmail.com>      *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "KPrintManagerConfigPlugin.h"
#include "ClassListWidget.h"

#include <QtPlugin>

#include <KGlobal>

static const KCatalogLoader loader(QLatin1String("printmanager"));

KPrintManagerConfigPlugin::KPrintManagerConfigPlugin(QObject *parent)
: QObject(parent)
{
}

bool KPrintManagerConfigPlugin::isContainer() const
{
    return false;
}

QIcon KPrintManagerConfigPlugin::icon() const
{
    return QIcon();
}

QString KPrintManagerConfigPlugin::group() const
{
    return QString();
}

QString KPrintManagerConfigPlugin::includeFile() const
{
    return "ClassListWidget.h";
}

QString KPrintManagerConfigPlugin::name() const
{
    return "ClassListWidget";
}

QString KPrintManagerConfigPlugin::toolTip() const
{
    return QString();
}

QString KPrintManagerConfigPlugin::whatsThis() const
{
    return QString();
}

QWidget * KPrintManagerConfigPlugin::createWidget(QWidget *parent)
{
    return new ClassListWidget(parent);
}

Q_EXPORT_PLUGIN2(printmanagerwidget, KPrintManagerConfigPlugin)

