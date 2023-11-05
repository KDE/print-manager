set(configureprinter_SRCS
    ConfigurePrinterInterface.cpp
    ConfigurePrinterInterface.h
    Debug.cpp
    Debug.h
    ModifyPrinter.cpp
    ModifyPrinter.h
    PrinterPage.cpp
    PrinterPage.h
    PrinterOptions.cpp
    PrinterOptions.h
    PrinterBehavior.cpp
    PrinterBehavior.h
    ConfigureDialog.cpp
    ConfigureDialog.h
    ConfigurePrinter.cpp
    ConfigurePrinter.h
    SelectMakeModelDialog.cpp
    SelectMakeModelDialog.h
    main.cpp
)

ki18n_wrap_ui(configureprinter_SRCS
    ModifyPrinter.ui
    PrinterOptions.ui
    PrinterBehavior.ui
)

qt_add_dbus_adaptor(configureprinter_SRCS
    org.kde.ConfigurePrinter.xml
    ConfigurePrinterInterface.h
    ConfigurePrinterInterface
)

add_executable(configure-printer ${configureprinter_SRCS})
target_link_libraries(configure-printer
    Qt::Core
    Qt::Widgets
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::DBusAddons
    KF6::ConfigCore
    Cups::Cups
    kcupslib
)

target_link_libraries(configure-printer
    Qt6::Core5Compat
    KF6::IconWidgets
)

install(TARGETS configure-printer DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.ConfigurePrinter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
