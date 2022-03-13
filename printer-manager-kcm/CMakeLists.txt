add_library(kcm_printer_manager MODULE printer.cpp)

target_link_libraries(kcm_printer_manager
    Qt::Gui
    Qt::Qml
    Qt::DBus
    KF5::CoreAddons
    KF5::ConfigGui
    KF5::QuickAddons
    KF5::I18n
    kcupslib
    )

install(TARGETS kcm_printer_manager DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)
kpackage_install_package(package kcm_printer_manager kcms)

install(FILES kcm_printer_manager.desktop DESTINATION ${KDE_INSTALL_APPDIR})
