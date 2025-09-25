# SPDX-FileCopyrightText: 2025 Mike Noe <noeerover@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

add_executable(plasma-print-queue)

ecm_add_qml_module(plasma-print-queue
    GENERATE_PLUGIN_SOURCE
    URI org.kde.plasma.printqueue
    SOURCES
        main.cpp
        PrintQueue.cpp
        PrintQueue.h
    QML_FILES
        qml/Main.qml
        qml/JobView.qml
        qml/NotAvailablePage.qml
)


target_link_libraries(plasma-print-queue PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    KF6::CoreAddons
    KF6::I18n
    KF6::I18nQml
    KF6::DBusAddons
    Plasma::Plasma
    KirigamiApp
    kcups
)

ecm_qt_declare_logging_category(plasma-print-queue
    HEADER pmqueue_log.h
    IDENTIFIER PMQUEUE
    CATEGORY_NAME org.kde.plasma.printmanager.printqueue
    DESCRIPTION "Plasma Print Queue Manager"
    EXPORT PMLOGS
)

install(TARGETS plasma-print-queue ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.plasma.printqueue.desktop DESTINATION ${KDE_INSTALL_APPDIR})
