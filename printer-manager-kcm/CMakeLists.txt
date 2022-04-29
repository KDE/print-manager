set(kcm_print_SRCS
    PrinterDelegate.cpp
    PrinterDescription.cpp
    PrintKCM.cpp
)

ki18n_wrap_ui(kcm_print_SRCS
    PrintKCM.ui
    PrinterDescription.ui
)

add_library(kcm_printer_manager MODULE ${kcm_print_SRCS})

target_link_libraries(kcm_printer_manager
    Qt::Core
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KCMUtils
    PkgConfig::CUPS
    kcupslib
)

install(TARGETS kcm_printer_manager DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets)
install(FILES kcm_printer_manager.desktop DESTINATION ${KDE_INSTALL_APPDIR})
