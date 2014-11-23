set(printmanager_config_SRCS
    KPrintManagerConfigPlugin.cpp
)

add_library(printmanagerwidget MODULE ${printmanager_config_SRCS})

target_link_libraries(printmanagerwidget
    KF5::WidgetsAddons
    Qt5::Designer
    kcupslib
)

# QML Stuff
plasma_install_package(package org.kde.plasma.printmanager)
