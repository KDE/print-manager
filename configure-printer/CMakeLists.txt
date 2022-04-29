set(configureprinter_SRCS
    ConfigurePrinterInterface.cpp
    Debug.cpp
    ModifyPrinter.cpp
    PrinterPage.cpp
    PrinterOptions.cpp
    PrinterBehavior.cpp
    ConfigureDialog.cpp
    ConfigurePrinter.cpp
    SelectMakeModelDialog.cpp
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
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    KF5::DBusAddons
    KF5::ConfigCore
    CUPS::CUPS
    kcupslib
)

install(TARGETS configure-printer DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.ConfigurePrinter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
