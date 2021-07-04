/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINT_MANAGER_QMLPLUGINS_H
#define PRINT_MANAGER_QMLPLUGINS_H

#include <QQmlExtensionPlugin>

class QmlPlugins : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

    public:
        void registerTypes(const char * uri) override;
};

#endif // PRINT_MANAGER_QMLPLUGINS_H
