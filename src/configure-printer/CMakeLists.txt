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

    ConfigurePrinterInterface.h
    Debug.h
    ModifyPrinter.h
    PrinterPage.h
    PrinterOptions.h
    PrinterBehavior.h
    ConfigureDialog.h
    ConfigurePrinter.h
    SelectMakeModelDialog.h
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
    Qt6::Core5Compat
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::DBusAddons
    KF6::ConfigCore
    KF6::IconWidgets
    Cups::Cups
    kcupslib
)

install(TARGETS configure-printer DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.ConfigurePrinter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
